#include <string.h>

#include <stm32f0xx_spi.h>

#include <cc85xx_ehif.h>
#include <stm32_spi.h>

/* EHIF Cmd Type Defines */
typedef enum spi_cmd_req_type_e
{
    CMD_NONE                  = 0x00,
    CMD_DI_GET_CHIP_INFO      = 0x1F,
    CMD_DI_GET_DEVICE_INFO    = 0x1E,
    
    CMD_EHC_EVT_MASK          = 0x1A,
    CMD_EHC_EVT_CLR           = 0x19,
    
    CMD_NVM_DO_SCAN           = 0x08,
    CMD_NVM_DO_JOIN           = 0x09,
    CMD_NVM_GET_STATUS        = 0x0A,
    CMD_NVM_ACH_SET_USAGE     = 0x0B,
    CMD_NVM_CONTROL_ENABLE    = 0x0C,
    CMD_NVM_CONTROL_SIGNAL    = 0x0D,
    
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
} SPI_CMD_REQ_TYPE_E;

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

#define EHIF_HEAD_SIZE  sizeof(SPI_HEAD_CMD_REQ_S)

static SPI_S gs_cc85xx_spi[2];

static void cc85xx_spi_set_enable(SPI_S *spi, uint8_t enable)
{
    __disable_irq();

    GPIO_WriteBit(spi->cs_gpio, spi->cs_pin, enable ? Bit_RESET : Bit_SET);
    if(enable) {
        while(GPIO_ReadInputDataBit(spi->spi_gpio, spi->pin_miso) == Bit_RESET);
    }
    SPI_Cmd(spi->spi_id, enable ? ENABLE : DISABLE);

    __enable_irq();
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

    memset(status, 0, sizeof(EHIF_STATUS_S));
    
    phead = (magic == EHIF_MAGIC_CMD_REQ) ? (void *)&cmd_head : (void *)&rw_head;

    /* Start Communicate */
    gs_cc85xx_spi[ehif->spi_dev].set_enable(&gs_cc85xx_spi[ehif->spi_dev], ENABLE);

    gs_cc85xx_spi[ehif->spi_dev].transfer(&gs_cc85xx_spi[ehif->spi_dev],
                                        phead, EHIF_HEAD_SIZE, status, sizeof(EHIF_STATUS_S));

    switch(magic) {
    case EHIF_MAGIC_CMD_REQ:
    case EHIF_MAGIC_WRITE:
        if(*len) {
            gs_cc85xx_spi[ehif->spi_dev].transfer(&gs_cc85xx_spi[ehif->spi_dev], data, *len, NULL, 0);
        }
        break;
    case EHIF_MAGIC_READBC:
        gs_cc85xx_spi[ehif->spi_dev].transfer(&gs_cc85xx_spi[ehif->spi_dev], NULL, 0, len, 2);
    case EHIF_MAGIC_READ:
        if(*len) {
            gs_cc85xx_spi[ehif->spi_dev].transfer(&gs_cc85xx_spi[ehif->spi_dev], NULL, 0, data, *len);
        }
        break;
    }

    /* End Communicate */
    gs_cc85xx_spi[ehif->spi_dev].set_enable(&gs_cc85xx_spi[ehif->spi_dev], DISABLE);

#if defined(DUMP_ENABLE)
    switch(cmd) {
    case CMD_RC_SET_DATA:
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
        break;
    }
#endif
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
}

static void ehif_EHC_EVT_CLR(CC85XX_EHIF_S *ehif, uint8_t val)
{
    _basic_CMD_REQ(ehif, CMD_EHC_EVT_CLR, sizeof(uint8_t), &val, &ehif->status);
}

static void ehif_NWM_CONTROL_ENABLE(CC85XX_EHIF_S *ehif, uint8_t enable)
{
    uint8_t sendbuf[2] = {0, enable};

    _basic_CMD_REQ(ehif, CMD_NVM_CONTROL_ENABLE, sizeof(sendbuf), sendbuf, &ehif->status);
}

static void ehif_NWM_CONTROL_SIGNAL(CC85XX_EHIF_S *ehif, uint8_t enable)
{
    uint8_t sendbuf[2] = {0, enable};

    _basic_CMD_REQ(ehif, CMD_NVM_CONTROL_SIGNAL, sizeof(sendbuf), sendbuf, &ehif->status);
}

static void ehif_NWM_GET_STATUS(CC85XX_EHIF_S *ehif, EHIF_NWM_GET_STATUS_S *nwm_status)
{
    _basic_CMD_REQ(ehif, CMD_NVM_GET_STATUS, 0, NULL, &ehif->status);
    _basic_READ(ehif, sizeof(EHIF_NWM_GET_STATUS_S), nwm_status, &ehif->status);
}

static void ehif_VC_SET_VOLUME(CC85XX_EHIF_S *ehif, EHIF_SET_VOLUME_S *set_volume)
{
    void *pdata = (void *)set_volume;

    _basic_CMD_REQ(ehif, CMD_VC_SET_VOLUME, sizeof(EHIF_SET_VOLUME_S), pdata, &ehif->status);
}

static void ehif_VC_GET_VOLUME(CC85XX_EHIF_S *ehif, EHIF_GET_VOLUME_S *get_volume, uint16_t *volume)
{
    _basic_CMD_REQ(ehif, CMD_VC_GET_VOLUME, 1, get_volume, &ehif->status);
    _basic_READ(ehif, sizeof(uint16_t), volume, &ehif->status);
}


void ehif_init(CC85XX_EHIF_S *ehif, CC85XX_DEV_TYPE_E spi_dev)
{
    ehif->spi_dev = spi_dev;

    gs_cc85xx_spi[ehif->spi_dev].dev        = ehif->spi_dev;
    gs_cc85xx_spi[ehif->spi_dev].cs_pin     = (spi_dev == DEV_TYPE_MIC) ? GPIO_Pin_14 : GPIO_Pin_15;
    gs_cc85xx_spi[ehif->spi_dev].init       = spi_init;
    gs_cc85xx_spi[ehif->spi_dev].set_enable = cc85xx_spi_set_enable;
    gs_cc85xx_spi[ehif->spi_dev].transfer   = spi_transfer;

    gs_cc85xx_spi[ehif->spi_dev].init(&gs_cc85xx_spi[ehif->spi_dev], SPI2);

    ehif->get_status            = ehif_GET_STATUS;
    ehif->di_get_device_info    = ehif_DI_GET_DEVICE_INFO;
    ehif->ehc_evt_clr           = ehif_EHC_EVT_CLR;
    ehif->nwm_control_enable    = ehif_NWM_CONTROL_ENABLE;
    ehif->nwm_control_signal    = ehif_NWM_CONTROL_SIGNAL;
    ehif->nwm_get_status        = ehif_NWM_GET_STATUS;
    ehif->vc_set_volume         = ehif_VC_SET_VOLUME;
    ehif->vc_get_volume         = ehif_VC_GET_VOLUME;
}
