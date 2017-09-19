#include <string.h>

#include <cc85xx_ehif.h>
#include <stm32_spi.h>
#include <debug.h>
#include <stm32_timer.h>
#include <vocal_common.h>

/* EHIF Cmd Type Defines */
typedef enum ehif_cmd_req_type_e
{
    CMD_NONE                  = 0x00,
    CMD_DI_GET_CHIP_INFO      = 0x1F,
    CMD_DI_GET_DEVICE_INFO    = 0x1E,
    
    CMD_EHC_EVT_MASK          = 0x1A,
    CMD_EHC_EVT_CLR           = 0x19,
    
    CMD_NWM_DO_SCAN           = 0x08,
    CMD_NWM_DO_JOIN           = 0x09,
    CMD_NWM_GET_STATUS        = 0x0A,
    CMD_NWM_ACH_SET_USAGE     = 0x0B,
    CMD_NWM_CONTROL_ENABLE    = 0x0C,
    CMD_NWM_CONTROL_SIGNAL    = 0x0D,
    
    CMD_DSC_TX_DATAGRAM       = 0x04,  
    CMD_DSC_RX_DATAGRAM       = 0x05,
    
    CMD_PM_SET_STATE          = 0x1C,
    CMD_PM_GET_DATA           = 0x1D,
    
    CMD_VC_GET_VOLUME         = 0x16,
    CMD_VC_SET_VOLUME         = 0x17,
    
    CMD_PS_RF_STATS           = 0x10,
    CMD_PS_AUDIO_STATS        = 0x11,
    
    CMD_CAL_SET_DATA          = 0x28,
    CMD_CAL_GET_DATA          = 0x29,
    
    CMD_NVS_GET_DATA          = 0x2B,
    CMD_NVS_SET_DATA          = 0x2C,

    CMD_RC_SET_DATA           = 0x2D,
    CMD_RC_GET_DATA           = 0x2E,
} EHIF_CMD_REQ_TYPE_E;

typedef enum ehif_magic_num_e {
    EHIF_MAGIC_CMD_REQ      = 0x3,      /* 0b11 */
    EHIF_MAGIC_WRITE        = 0x8,      /* 0b1000 */
    EHIF_MAGIC_READ         = 0x9,      /* 0b1001 */
    EHIF_MAGIC_READBC       = 0xa,      /* 0b1010 */
} EHIF_MAGIC_NAM_E;

/* SPI Data Head Defines */
typedef struct spi_head_CMD_REQ_s
{
    uint16_t    data_len            :8;
    uint16_t    cmd_type            :6;
    uint16_t    magic_num           :2;
} SPI_HEAD_CMD_REQ_S;

typedef struct spi_head_READ_WRITE_s
{
    uint16_t    data_len            :12;
    uint16_t    magic_num           :4;
} SPI_HEAD_READ_WRITE_S;

typedef struct ehif_rc_get_head_s
{
    uint8_t     cmd_count   :3;
    uint8_t     keyb_count  :3;
    uint8_t     ext_sel     :2;
} EHIF_RC_GET_HEAD_S;

typedef struct ehif_rc_get_data_s
{
    EHIF_RC_GET_HEAD_S  head;
    uint8_t             data[12];
} EHIF_RC_GET_DATA_S;


#define EHIF_HEAD_SIZE  sizeof(SPI_HEAD_CMD_REQ_S)

static SPI_S gs_cc85xx_spi[2];

static void cc85xx_spi_set_enable(SPI_S *spi, uint8_t enable)
{
    GPIO_WriteBit(spi->gpio_cs, spi->pin_cs, enable ? Bit_RESET : Bit_SET);
    delay_ms(1);
#if 1
    if(enable) {
        while(GPIO_ReadInputDataBit(spi->gpio_spi, spi->pin_miso) == Bit_RESET);
    }
#endif
    //SPI_Cmd(spi->spi_id, enable ? ENABLE : DISABLE);
}

static void _basic_operation(CC85XX_EHIF_S *ehif, EHIF_MAGIC_NAM_E magic,
                        uint16_t cmd, uint16_t *len, void *data, EHIF_STATUS_S *status)
{
    SPI_HEAD_CMD_REQ_S      cmd_head    = {0};
    SPI_HEAD_READ_WRITE_S   rw_head     = {0};
    void                    *phead      = NULL;

    cmd_head.cmd_type   = cmd;
    cmd_head.data_len   = rw_head.data_len  = *len;
    cmd_head.magic_num  = rw_head.magic_num = magic;
    
    *(uint16_t *)&cmd_head  = TONET16(*(uint16_t *)&cmd_head);
    *(uint16_t *)&rw_head   = TONET16(*(uint16_t *)&rw_head);

    memset(status, 0, sizeof(EHIF_STATUS_S));
    
    phead = (magic == EHIF_MAGIC_CMD_REQ) ? (void *)&cmd_head : (void *)&rw_head;

    /* Start Communicate */
    gs_cc85xx_spi[ehif->dev_type].set_enable(&gs_cc85xx_spi[ehif->dev_type], ENABLE);

    gs_cc85xx_spi[ehif->dev_type].transfer(&gs_cc85xx_spi[ehif->dev_type],
                                        phead, EHIF_HEAD_SIZE, status, sizeof(EHIF_STATUS_S));
    switch(magic) {
    case EHIF_MAGIC_CMD_REQ:
    case EHIF_MAGIC_WRITE:
        if(*len) {
            gs_cc85xx_spi[ehif->dev_type].transfer(&gs_cc85xx_spi[ehif->dev_type], data, *len, NULL, 0);
        }
        break;
    case EHIF_MAGIC_READBC:
        gs_cc85xx_spi[ehif->dev_type].transfer(&gs_cc85xx_spi[ehif->dev_type], NULL, 0, len, 2);
    case EHIF_MAGIC_READ:
        if(*len) {
            gs_cc85xx_spi[ehif->dev_type].transfer(&gs_cc85xx_spi[ehif->dev_type], NULL, 0, data, *len);
        }
        break;
    }

    /* End Communicate */
    gs_cc85xx_spi[ehif->dev_type].set_enable(&gs_cc85xx_spi[ehif->dev_type], DISABLE);
#if 0
    if(tmpdebug) {
        datadump("s", phead, EHIF_HEAD_SIZE);
        datadump("r", status, sizeof(EHIF_STATUS_S));

        if(*len) {
            switch(magic) {
            case EHIF_MAGIC_CMD_REQ:
            case EHIF_MAGIC_WRITE:
                datadump("s", data, *len);
                break;
            case EHIF_MAGIC_READBC:   
            case EHIF_MAGIC_READ:
                datadump("r", data, *len);
                break;
            }
        }
    }
#endif
#if defined(DUMP_ENABLE3)
    datadump("s", phead, EHIF_HEAD_SIZE);
    datadump("r", status, sizeof(EHIF_STATUS_S));

    if(*len) {
        switch(magic) {
        case EHIF_MAGIC_CMD_REQ:
        case EHIF_MAGIC_WRITE:
            datadump("s", data, *len);
            break;
        case EHIF_MAGIC_READBC:   
        case EHIF_MAGIC_READ:
            datadump("r", data, *len);
            break;
        }
    }
#endif

    *(uint16_t *)status   = TOHOST16(*(uint16_t *)status);
    memcpy(&ehif->status, status, sizeof(EHIF_STATUS_S));
}

#define _basic_CMD_REQ(ehif, cmd, len, pdata, pstatus) \
{                                                                                   \
    uint16_t l = (len);                                                             \
    _basic_operation((ehif), EHIF_MAGIC_CMD_REQ, (cmd), &l, (pdata), (pstatus));    \
}

#define _basic_WRITE(ehif, len, pdata, pstatus) \
{                                                                                   \
    uint16_t l = (len);                                                             \
    _basic_operation((ehif), EHIF_MAGIC_WRITE, CMD_NONE, &l, (pdata), (pstatus));   \
}

#define _basic_GET_STATUS(ehif, pstatus) \
{                                                                                   \
    uint16_t l = 0;                                                                 \
    _basic_operation((ehif), EHIF_MAGIC_WRITE, 0, &l, 0, (pstatus));                \
}

#define _basic_READ(ehif, len, pdata, pstatus) \
{                                                                                   \
    uint16_t l = (len);                                                             \
    _basic_operation((ehif), EHIF_MAGIC_READ, CMD_NONE, &l, (pdata), (pstatus));    \
}

#define _basic_READBC(ehif, plen, pdata, pstatus) \
    _basic_operation((ehif), EHIF_MAGIC_READBC, CMD_NONE, (plen), (pdata), (pstatus))

static void ehif_GET_STATUS(CC85XX_EHIF_S *ehif)
{
    _basic_GET_STATUS(ehif, &ehif->status);
}

static void ehif_DI_GET_DEVICE_INFO(CC85XX_EHIF_S *ehif, EHIF_DEV_INFO_S *dev_info)
{
    _basic_CMD_REQ(ehif, CMD_DI_GET_DEVICE_INFO, 0, NULL, &ehif->status);
    _basic_READ(ehif, sizeof(EHIF_DEV_INFO_S), (uint8_t *)dev_info, &ehif->status);
    
    dev_info->device_id = TOHOST32(dev_info->device_id);
    dev_info->manf_id   = TOHOST32(dev_info->manf_id);
    dev_info->prod_id   = TOHOST32(dev_info->prod_id);
}

static void ehif_EHC_EVT_CLR(CC85XX_EHIF_S *ehif, uint8_t val)
{
    _basic_CMD_REQ(ehif, CMD_EHC_EVT_CLR, sizeof(uint8_t), &val, &ehif->status);
}

static void ehif_NWM_CONTROL_ENABLE(CC85XX_EHIF_S *ehif, uint8_t enable)
{
    uint8_t sendbuf[2] = {0, enable};

    _basic_CMD_REQ(ehif, CMD_NWM_CONTROL_ENABLE, sizeof(sendbuf), sendbuf, &ehif->status);
}

static void ehif_NWM_CONTROL_SIGNAL(CC85XX_EHIF_S *ehif, uint8_t enable)
{
    uint8_t         sendbuf[2] = {0, enable};
    EHIF_STATUS_S   status;
    
    while(!ehif->status.cmdreq_rdy) {
        delay_ms(10);
        _basic_GET_STATUS(ehif, &status);
    }
    _basic_CMD_REQ(ehif, CMD_NWM_CONTROL_SIGNAL, sizeof(sendbuf), sendbuf, &ehif->status);
    DEBUG("EHIF   : %s Signal %s\n", ehif->dev_type ? "MIC" : "SPK", enable ? "enable" : "disable");
}

static void ehif_NWM_GET_STATUS(CC85XX_EHIF_S *ehif, EHIF_NWM_GET_STATUS_S *nwm_status)
{
    EHIF_STATUS_S   status;
    
    while(!ehif->status.cmdreq_rdy) {
        delay_ms(10);
        _basic_GET_STATUS(ehif, &status);
    }
    
    _basic_CMD_REQ(ehif, CMD_NWM_GET_STATUS, 0, NULL, &ehif->status);
    _basic_READ(ehif, sizeof(EHIF_NWM_GET_STATUS_S), nwm_status, &ehif->status);
}

static void ehif_VC_SET_VOLUME(CC85XX_EHIF_S *ehif, EHIF_SET_VOLUME_S *set_volume)
{
    void *pdata = (void *)set_volume;

    set_volume->volume = TONET16(set_volume->volume);
    _basic_CMD_REQ(ehif, CMD_VC_SET_VOLUME, sizeof(EHIF_SET_VOLUME_S), pdata, &ehif->status);
}

static void ehif_VC_GET_VOLUME(CC85XX_EHIF_S *ehif, EHIF_GET_VOLUME_S *get_volume, uint16_t *volume)
{
    _basic_CMD_REQ(ehif, CMD_VC_GET_VOLUME, 1, get_volume, &ehif->status);
    _basic_READ(ehif, sizeof(uint16_t), volume, &ehif->status);
}

static void ehif_RC_GET_DATA(CC85XX_EHIF_S *ehif, uint8_t slot, uint8_t *data, uint8_t *count)
{
    EHIF_RC_GET_DATA_S get_data = {0};

    _basic_CMD_REQ(ehif, CMD_RC_GET_DATA, sizeof(slot), &slot, &ehif->status);
    _basic_READ(ehif, sizeof(EHIF_RC_GET_DATA_S), &get_data, &ehif->status);

    *count = (get_data.head.cmd_count > 12) ? 12 : get_data.head.cmd_count;

    memcpy(data, get_data.data, *count);
}

void ehif_init(CC85XX_EHIF_S *ehif, VOCAL_DEV_TYPE_E dev_type)
{
    ehif->dev_type = dev_type;

    gs_cc85xx_spi[ehif->dev_type].dev           = ehif->dev_type;
    gs_cc85xx_spi[ehif->dev_type].pin_cs        = (dev_type == DEV_TYPE_SPK) ? SPK_CS_PIN   : MIC_CS_PIN;
    gs_cc85xx_spi[ehif->dev_type].gpio_cs       = (dev_type == DEV_TYPE_SPK) ? SPK_CS_GPIO  : MIC_CS_GPIO;
    gs_cc85xx_spi[ehif->dev_type].init          = spi_init;
    gs_cc85xx_spi[ehif->dev_type].set_enable    = cc85xx_spi_set_enable;

    gs_cc85xx_spi[ehif->dev_type].init(&gs_cc85xx_spi[ehif->dev_type], SPI1);

    ehif->get_status            = ehif_GET_STATUS;
    ehif->di_get_device_info    = ehif_DI_GET_DEVICE_INFO;
    ehif->ehc_evt_clr           = ehif_EHC_EVT_CLR;
    ehif->nwm_control_enable    = ehif_NWM_CONTROL_ENABLE;
    ehif->nwm_control_signal    = ehif_NWM_CONTROL_SIGNAL;
    ehif->nwm_get_status        = ehif_NWM_GET_STATUS;
    ehif->vc_set_volume         = ehif_VC_SET_VOLUME;
    ehif->vc_get_volume         = ehif_VC_GET_VOLUME;
    ehif->rc_get_data           = ehif_RC_GET_DATA;

    ehif->get_status(ehif);
}

