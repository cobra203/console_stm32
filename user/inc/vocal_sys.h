#ifndef _VOCAL_SYS_H_
#define _VOCAL_SYS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>

struct cc85xx_dev_s;
struct cc85xx_pair_s;

typedef struct vocal_sys_s
{
    uint8_t                 sys_updata;
    struct cc85xx_dev_s     *mic_dev;
    struct cc85xx_dev_s     *spk_dev;
    struct cc85xx_pair_s    *pair;
} VOCAL_SYS_S;

#ifdef __cplusplus
}
#endif

#endif /* _VOCAL_SYS_H_ */

