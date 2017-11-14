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
    int8_t      volume;
    uint8_t     mute;
    uint8_t     ach;
    uint8_t     slot;
    uint8_t     cmd_rc;
    uint8_t     cmd_pc;
} NWK_DEV_INFO_S;

typedef struct nwk_info_s
{
    uint32_t    device_id;
    uint8_t     ach_used;
    uint8_t     slot;
} NWK_INFO_S;

typedef enum rang_of_set_volume_s
{
    RANG_SET_ALL_DEV,
    RANG_SET_RC_ONLY,
    RANG_SET_PC_ONLY,
} RANG_OF_SET_VOLUME_S;

typedef struct pair_status_s
{
    uint8_t     dev_idx;
    uint8_t     task;
} PAIR_STATUS_S;

typedef struct cc85xx_dev_s
{
    VOCAL_SYS_S     *vocal_sys;
    uint8_t         nwk_enable;
    uint8_t         nwk_stable;
    PAIR_STATUS_S   pair_status;
    NWK_DEV_INFO_S  nwk_dev[MAX_DEV_NUM];
    NWK_INFO_S      new_nwk_info[MAX_DEV_NUM];
    CC85XX_EHIF_S   ehif;
    void            (*init)             (struct cc85xx_dev_s *);
    void            (*nwk_chg_detect)   (struct cc85xx_dev_s *, VOCAL_DEV_TYPE_E);
    void            (*rc_cmd_detect)    (struct cc85xx_dev_s *);
    void            (*execute)          (struct cc85xx_dev_s *, VOCAL_DEV_TYPE_E, RANG_OF_SET_VOLUME_S);
    void            (*nwk_pairing)      (struct cc85xx_dev_s *, uint8_t);
} CC85XX_DEV_S;

//void cc85xx_dev_init(CC85XX_DEV_S *dev);

void spk_dev_init(VOCAL_SYS_S *sys_status);
void mic_dev_init(VOCAL_SYS_S *sys_status);

void spk_detect(VOCAL_SYS_S *vocal_sys);
void mic_detect(VOCAL_SYS_S *sys_status);



#ifdef __cplusplus
}
#endif

#endif /* _CC85XX_DEV_H_ */

