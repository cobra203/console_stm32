#ifndef _VOCAL_RECORD_H_
#define _VOCAL_RECORD_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>
#include <vocal_sys.h>

typedef struct record_dev_id_s
{
    uint32_t    device_id   :30;
    uint32_t    index       :2;
} RECORD_DEV_ID_S;

typedef struct record_evt_s
{
    uint8_t     rcd_evt_dev_id_chg  :1;
    uint8_t     rcd_evt_volume_chg  :1;
} RECORD_EVT_S;

typedef struct vocal_record_s
{
    VOCAL_SYS_S         *vocal_sys;
    RECORD_EVT_S        evt;
    uint8_t             active;
    int                 addr_idx_id;
    int                 addr_idx_vl;
    RECORD_DEV_ID_S     spk_id[SPK_DEV_NUM];
    RECORD_DEV_ID_S     mic_id[MIC_DEV_NUM];
    RECORD_VOLUME_S     spk_vl;
    RECORD_VOLUME_S     mic_vl[MIC_DEV_NUM];
    
    void        (*dev_id_init)  (struct vocal_record_s *);
    void        (*volume_init)  (struct vocal_record_s *);
    int         (*add)          (struct vocal_record_s *, const uint32_t, const VOCAL_DEV_TYPE_E, RECORD_VOLUME_S *, int *);
    int         (*get)          (struct vocal_record_s *, const uint32_t, const VOCAL_DEV_TYPE_E, RECORD_VOLUME_S *, int *);
    int         (*set)          (struct vocal_record_s *, const uint32_t, const VOCAL_DEV_TYPE_E, const RECORD_VOLUME_S *, int *);
    int         (*commit)       (struct vocal_record_s *);
} VOCAL_RECORD_S;

void record_init(VOCAL_SYS_S *vocal_sys);

#ifdef __cplusplus
}
#endif

#endif /* _VOCAL_RECORD_H_ */

