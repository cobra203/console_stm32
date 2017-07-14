#ifndef _CC85XX_EHIF_H_
#define _CC85XX_EHIF_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>

#define MIC_MAX_NUM 4
#define SPK_MAX_NUM 1

/* EHIF Status Word Define */
typedef struct ehif_status_word_s
{
    uint16_t    evt_sr_chg          :1;
    uint16_t    evt_nwk_chg         :1;
    uint16_t    evt_ps_chg          :1;
    uint16_t    evt_vol_chg         :1;
    uint16_t    evt_spi_error       :1;
    uint16_t    evt_dsc_reset       :1;
    uint16_t    evt_dsc_tx_avail    :1;
    uint16_t    evt_dsc_rx_avail    :1;
    uint16_t    wasp_conn           :1;
    uint16_t    pwr_state           :3;
    uint16_t                        :3;
    uint16_t    cmdreq_rdy          :1;
} EHIF_STATUS_S;

typedef struct ehif_device_info_s
{
    uint32_t    device_id;
    uint32_t    manf_id;
    uint32_t    prod_id;
} EHIF_DEV_INFO_S;

typedef struct ehif_set_volume_s
{
    uint8_t     is_local    :1;
    uint8_t     is_in_vol   :1;
    uint8_t                 :6;
    uint8_t     log_channel :4;
    uint8_t     set_op      :2;
    uint8_t     mute_op     :2;
    uint16_t    volume      :16;
} EHIF_SET_VOLUME_S;

typedef struct ehif_get_volume_s
{
    uint8_t     is_local    :1;
    uint8_t     is_in_vol   :1;
    uint8_t                 :6;
} EHIF_GET_VOLUME_S;

typedef struct ehif_nwm_get_status_s
{
    uint8_t     byte0_to_4[5];
    uint8_t     dev_data[16 * MIC_MAX_NUM];
} EHIF_NWM_GET_STATUS_S;

typedef enum cc85xx_dev_type_e
{
    DEV_TYPE_MIC = 0,
    DEV_TYPE_SPK,  
} CC85XX_DEV_TYPE_E;

typedef struct cc85xx_ehif_s
{
    CC85XX_DEV_TYPE_E   spi_dev;
    EHIF_STATUS_S       status;
    
    void    (*init)                 (struct cc85xx_ehif_s *, CC85XX_DEV_TYPE_E);
    void    (*get_status)           (struct cc85xx_ehif_s *);
    void    (*di_get_device_info)   (struct cc85xx_ehif_s *, EHIF_DEV_INFO_S *);
    void    (*ehc_evt_clr)          (struct cc85xx_ehif_s *, uint8_t);
    void    (*nwm_control_enable)   (struct cc85xx_ehif_s *, uint8_t);
    void    (*nwm_control_signal)   (struct cc85xx_ehif_s *, uint8_t);
    void    (*nwm_get_status)       (struct cc85xx_ehif_s *, EHIF_NWM_GET_STATUS_S *);
    void    (*vc_set_volume)        (struct cc85xx_ehif_s *, EHIF_SET_VOLUME_S *);
    void    (*vc_get_volume)        (struct cc85xx_ehif_s *, EHIF_GET_VOLUME_S *, uint16_t *);
} CC85XX_EHIF_S;

void ehif_init(CC85XX_EHIF_S *ehif, CC85XX_DEV_TYPE_E spi_dev);

#ifdef __cplusplus
}
#endif

#endif /* _CC85XX_EHIF_H_ */