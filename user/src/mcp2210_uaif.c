#include <mcp2210_uaif.h>
#include <stm32_spi.h>
#include <vocal_common.h>
#include <string.h>
#include <debug.h>

typedef enum uaif_magic_num_e {
    UAIF_MAGIC_NULL         = 0,
    UAIF_MAGIC_CMD_REQ      = 0x3,      /* 0b11 */
} UAIF_MAGIC_NAM_E;

typedef struct pc_set_volume_args_s
{
    uint8_t         idx;
    RECORD_VOLUME_S volume;
} PC_SET_VOLUME_ARGS_S;

typedef struct pc_get_volume_args_s
{
    uint8_t         active;
    RECORD_VOLUME_S volume[SPK_CFG_NUM + MIC_CFG_NUM];
} PC_GET_VOLUME_ARGS_S;

typedef struct pc_nwm_pairing_args_s
{
    uint8_t         type;
} PC_NWM_PAIRING_ARGS_S;

static SPI_S gs_mcp2210_spi;

static int _uaif_check_cmd_req(UAIF_HEAD_CMD_REQ_S *cmd)
{
    if(cmd->magic_num != UAIF_MAGIC_CMD_REQ) {
        return STM_FAILURE;
    }
    
    switch(cmd->cmd_type) {
    case CMD_GET_VOLUME:
        if(cmd->data_len != sizeof(PC_GET_VOLUME_ARGS_S)) {
            return STM_FAILURE;
        }
        break;
    case CMD_SET_VOLUME:
        if(cmd->data_len != sizeof(PC_SET_VOLUME_ARGS_S)) {
            return STM_FAILURE;
        }
        break;
    case CMD_NWM_CONTROL_SIGNAL:
        if(cmd->data_len != sizeof(PC_NWM_PAIRING_ARGS_S)) {
            return STM_FAILURE;
        }
        break;
    default:
        return STM_FAILURE;
    }

    return STM_SUCCESS;
}

static int uaif_get_cmd_type(MCP2210_UAIF_S *uaif, UAIF_CMD_REQ_TYPE_E *cmd_type)
{
    UAIF_HEAD_CMD_REQ_S     cmd_head = {0};
    uint16_t                status = TONET16(*(uint16_t *)&uaif->status);

    if(STM_SUCCESS != gs_mcp2210_spi.transfer(&gs_mcp2210_spi,
                            &status, sizeof(UAIF_STATUS_S), (void *)&cmd_head, sizeof(cmd_head))) {
        return STM_FAILURE;
    }

    *(uint16_t *)&cmd_head = TOHOST16(*(uint16_t *)&cmd_head);
    *cmd_type = cmd_head.cmd_type;
    
    return _uaif_check_cmd_req(&cmd_head);
}

static int uaif_SET_VOLUME(MCP2210_UAIF_S *uaif, int *cfg_idx, RECORD_VOLUME_S *volume)
{
    PC_SET_VOLUME_ARGS_S args = {0};


    tmpdebug = 0;
    if(STM_SUCCESS != gs_mcp2210_spi.transfer(&gs_mcp2210_spi, NULL, 0, (void *)&args, sizeof(PC_SET_VOLUME_ARGS_S))) {
        tmpdebug = 0;
        return STM_FAILURE;
    }
    tmpdebug = 0;

    *cfg_idx = args.idx;
    memcpy(volume, &args.volume, sizeof(RECORD_VOLUME_S));
    
    return STM_SUCCESS;
}

static int uaif_GET_VOLUME(MCP2210_UAIF_S *uaif, uint8_t active, const RECORD_VOLUME_S *volumes)
{
    PC_GET_VOLUME_ARGS_S    args;
    uint16_t                cmdreq_rdy;

    args.active = active;
    memcpy(&args.volume[0], volumes, sizeof(RECORD_VOLUME_S) * (SPK_CFG_NUM + MIC_CFG_NUM));

    tmpdebug = 0;
    if(STM_SUCCESS != gs_mcp2210_spi.transfer(&gs_mcp2210_spi, (void *)&args, sizeof(PC_GET_VOLUME_ARGS_S), NULL, 0)) {
        tmpdebug = 0;
        return STM_FAILURE;
    }
    tmpdebug = 0;

    cmdreq_rdy              = uaif->status.cmdreq_rdy;
    uaif->status            = uaif->status_commit;
    uaif->status.cmdreq_rdy = cmdreq_rdy;

    return STM_SUCCESS;
}

static int uaif_NWM_CONTROL_SIGNAL(MCP2210_UAIF_S *uaif, uint8_t *type)
{
    PC_NWM_PAIRING_ARGS_S   args;

    tmpdebug = 0;
    if(STM_SUCCESS != gs_mcp2210_spi.transfer(&gs_mcp2210_spi, NULL, 0, (void *)&args, sizeof(PC_NWM_PAIRING_ARGS_S))) {
        tmpdebug = 0;
        return STM_FAILURE;
    }
    tmpdebug = 0;

    *type = args.type;

    return STM_SUCCESS;
}


void uaif_init(MCP2210_UAIF_S *uaif, void (*process)(void *), void *args)
{

    gs_mcp2210_spi.init_slave_itc   = spi_init_slave_itc;
    gs_mcp2210_spi.init             = spi_init;
    gs_mcp2210_spi.pin_cs           = MCP_CS_PIN;
    gs_mcp2210_spi.gpio_cs          = MCP_CS_GPIO;

    uaif->get_cmd_type  = uaif_get_cmd_type;
    uaif->set_volume    = uaif_SET_VOLUME;
    uaif->get_volume    = uaif_GET_VOLUME;
    uaif->nwm_pairing   = uaif_NWM_CONTROL_SIGNAL;

    gs_mcp2210_spi.init_slave_itc(&gs_mcp2210_spi, process, args);
    gs_mcp2210_spi.init(&gs_mcp2210_spi, SPI2);
}

