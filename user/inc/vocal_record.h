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

typedef struct record_volume_s
{
    uint8_t     volume      :7;
    uint8_t     mute        :1;
} RECORD_VOLUME_S;

typedef struct vocal_record_s
{
    VOCAL_SYS_S         *vocal_sys;
    int                 addr_idx_id;
    int                 addr_idx_vl;
    RECORD_DEV_ID_S     mic_id[MIC_DEV_NUM];
    RECORD_DEV_ID_S     spk_id[SPK_DEV_NUM];
    RECORD_VOLUME_S     mic_vl[MIC_DEV_NUM];
    RECORD_VOLUME_S     spk_vl;
    
    void        (*dev_id_init)      (struct vocal_record_s *);
    void        (*volume_init)      (struct vocal_record_s *);
    void        (*dev_id_write)     (struct vocal_record_s *);
    void        (*volume_write)     (struct vocal_record_s *);
} VOCAL_RECORD_S;

void record_init(VOCAL_SYS_S *vocal_sys);

#ifdef __cplusplus
}
#endif

#endif /* _VOCAL_RECORD_H_ */

