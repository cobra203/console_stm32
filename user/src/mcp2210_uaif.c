#include <mcp2210_uaif.h>
#include <stm32_spi.h>
#include <vocal_common.h>
#include <string.h>

typedef enum uaif_magic_num_e {
    UAIF_MAGIC_NULL         = 0,
    UAIF_MAGIC_CMD_REQ      = 0x3,      /* 0b11 */
    UAIF_MAGIC_WRITE        = 0x8,      /* 0b1000 */
    UAIF_MAGIC_READ         = 0x9,      /* 0b1001 */
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

static SPI_S gs_mcp2210_spi;

static int _uaif_check_cmd_req(UAIF_HEAD_CMD_REQ_S *cmd)
{
    if(cmd->magic_num != UAIF_MAGIC_CMD_REQ) {
        return STM_FAILURE;
    }
    
    switch(cmd->cmd_type) {
    case CMD_GET_VOLUME:
        if(cmd->data_len) {
            return STM_FAILURE;
        }
        break;
    case CMD_SET_VOLUME:
        if(cmd->data_len != sizeof(PC_SET_VOLUME_ARGS_S)) {
            return STM_FAILURE;
        }
        break;
    default:
        return STM_FAILURE;
    }

    return STM_SUCCESS;
}

static int _uaif_check_rw_head(UAIF_HEAD_READ_WRITE_S *rw)
{
    if(rw->magic_num != UAIF_MAGIC_WRITE && rw->magic_num != UAIF_MAGIC_READ) {
        return STM_FAILURE;
    }

    if(rw->data_len != sizeof(PC_GET_VOLUME_ARGS_S)) {
        return STM_FAILURE;
    }

    return STM_SUCCESS;
}

static int uaif_get_cmd_type(MCP2210_UAIF_S *uaif, UAIF_CMD_REQ_TYPE_E *cmd_type)
{
    UAIF_HEAD_CMD_REQ_S     cmd_head = {0};
    int                     ret;

    *cmd_type    = CMD_NULL;
    
    gs_mcp2210_spi.transfer(&gs_mcp2210_spi, &uaif->status, sizeof(UAIF_STATUS_S), (void *)&cmd_head, sizeof(cmd_head));


    if(STM_SUCCESS == _uaif_check_cmd_req(&cmd_head)) {
        *(uint16_t *)cmd_type = cmd_head.cmd_type;
        if(!cmd_head.data_len) {
            gs_mcp2210_spi.transfer(&gs_mcp2210_spi, NULL, 0, NULL, 0);
        }
        ret = STM_SUCCESS;
    }


    else {
        gs_mcp2210_spi.transfer(&gs_mcp2210_spi, NULL, 0, NULL, 0);
        ret = STM_FAILURE;
    }

    return ret;
}

static void uaif_SET_VOLUME(MCP2210_UAIF_S *uaif, int *idx, RECORD_VOLUME_S *volume)
{
    PC_SET_VOLUME_ARGS_S args = {0};
    
    gs_mcp2210_spi.transfer(&gs_mcp2210_spi, NULL, 0, (void *)&args, sizeof(PC_SET_VOLUME_ARGS_S));
    gs_mcp2210_spi.transfer(&gs_mcp2210_spi, NULL, 0, NULL, 0);

    *idx = args.idx;
    memcpy(volume, &args.volume, sizeof(RECORD_VOLUME_S));
}

static int uaif_GET_VOLUME(MCP2210_UAIF_S *uaif, int active, const RECORD_VOLUME_S *volumes)
{
    PC_GET_VOLUME_ARGS_S    args;
    UAIF_HEAD_READ_WRITE_S  rw_head = {0};
    int                     ret = STM_FAILURE;

    gs_mcp2210_spi.transfer(&gs_mcp2210_spi, &uaif->status, sizeof(UAIF_STATUS_S), (void *)&rw_head, sizeof(rw_head));

    if(STM_SUCCESS == _uaif_check_rw_head(&rw_head)) {
        args.active = active;
        memcpy(&args.volume[0], volumes, sizeof(RECORD_VOLUME_S) * (SPK_CFG_NUM + MIC_CFG_NUM));
        
        gs_mcp2210_spi.transfer(&gs_mcp2210_spi, (void *)&args, sizeof(PC_GET_VOLUME_ARGS_S), NULL, 0);
        ret = STM_SUCCESS;
    }
    gs_mcp2210_spi.transfer(&gs_mcp2210_spi, NULL, 0, NULL, 0);

    return ret;
}

void uaif_init(MCP2210_UAIF_S *uaif, void (*process)(void *), void *args)
{

    gs_mcp2210_spi.init_slave_itc   = spi_init_slave_itc;
    gs_mcp2210_spi.init             = spi_init;
    gs_mcp2210_spi.cs_pin           = GPIO_Pin_4;

    uaif->get_cmd_type  = uaif_get_cmd_type;
    uaif->set_volume    = uaif_SET_VOLUME;
    uaif->get_volume    = uaif_GET_VOLUME;

    gs_mcp2210_spi.init_slave_itc(&gs_mcp2210_spi, process, args);
    gs_mcp2210_spi.init(&gs_mcp2210_spi, SPI2);
}

