#ifndef _VOCAL_SYS_H_
#define _VOCAL_SYS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>
#include <vocal_common.h>

struct cc85xx_dev_s;
struct cc85xx_pair_s;
struct mcp2210_dev_s;
struct vocal_record_s;
struct vocal_led_s;

typedef struct sys_evt_s
{
    uint8_t req_pairing         :1;
    uint8_t                     :3;
    uint8_t req_sync_spk_nwk    :1;
    uint8_t req_sync_mic_nwk    :1;
    uint8_t req_sync_rc_cmd     :1;
    uint8_t req_sync_pc_cmd     :1;
} SYS_EVT_S;

typedef struct vocal_sys_s
{
    SYS_EVT_S               sys_evt;
    struct cc85xx_dev_s     *mic_dev;
    struct cc85xx_dev_s     *spk_dev; 
    struct cc85xx_pair_s    *pair;
    struct mcp2210_dev_s    *mcp_dev;
    struct vocal_record_s   *record;
    struct vocal_led_s      *led;
    void                    (*sync_nwk_dev)     (struct vocal_sys_s *, VOCAL_DEV_TYPE_E);
    int                     (*sync_rc_cmd)      (struct vocal_sys_s *);
    void                    (*sync_pc_cmd)      (struct vocal_sys_s *);
    void                    (*nwk_pairing)      (struct vocal_sys_s *);
} VOCAL_SYS_S;

void vocal_nwk_pairing_callback(void *args);

void vocal_init(VOCAL_SYS_S *vocal);
void vocal_working(VOCAL_SYS_S *vocal);


#ifdef __cplusplus
}
#endif

#endif /* _VOCAL_SYS_H_ */

