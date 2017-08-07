#ifndef _VOCAL_SYS_H_
#define _VOCAL_SYS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>

#define MIC_DEV_NUM     2
#define SPK_DEV_NUM     4

struct cc85xx_dev_s;
struct cc85xx_pair_s;
struct vocal_record_s;
struct vocal_led_s;

typedef struct vocal_sys_s
{
    uint8_t                 sys_updata;
    struct cc85xx_dev_s     *mic_dev;
    struct cc85xx_dev_s     *spk_dev;
    struct cc85xx_pair_s    *pair;
    struct vocal_record_s   *record;
    struct vocal_led_s      *led;
} VOCAL_SYS_S;

#ifdef __cplusplus
}
#endif

#endif /* _VOCAL_SYS_H_ */

