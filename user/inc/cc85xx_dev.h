#ifndef _CC85XX_DEV_H_
#define _CC85XX_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cc85xx_ehif.h>
#include <vocal_sys.h>

typedef enum remote_control_cmd_e
{
    OUTPUT_VOLUME_INCREMENT = 2,
    OUTPUT_VOLUME_DECREASE  = 3,
    OUTPUT_VOLUME_MUTE      = 4,
} REMOTE_CONTROL_CMD_E;

typedef struct nwk_dev_info_s
{
    uint32_t    device_id;
    uint8_t     ach;
    uint8_t     slot;
    uint8_t     record;
    uint8_t     cmd_rc;
    uint8_t     cmd_pc;
    uint8_t     mute;
    int16_t     volume;
} NWK_DEV_INFO_S;

typedef struct cc85xx_dev_s
{
    VOCAL_SYS_S     *vocal_sys;
    uint8_t         nwk_enable;
    uint8_t         nwk_stable;
    NWK_DEV_INFO_S  nwk_dev[MIC_MAX_NUM];
    CC85XX_EHIF_S   ehif;
    void            (*init)     (struct cc85xx_dev_s *);
} CC85XX_DEV_S;

//void cc85xx_dev_init(CC85XX_DEV_S *dev);

void mic_dev_init(VOCAL_SYS_S *sys_status);
void spk_dev_init(VOCAL_SYS_S *sys_status);
void mic_detect(VOCAL_SYS_S *sys_status);



#ifdef __cplusplus
}
#endif

#endif /* _CC85XX_DEV_H_ */

