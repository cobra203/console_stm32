#ifndef _CC85XX_PAIR_H_
#define _CC85XX_PAIR_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <button.h>
#include <vocal_sys.h>


typedef struct cc85xx_pair_s
{
    VOCAL_SYS_S     *vocal_sys;
    BUTTON_S        btn_pair[2];
} CC85XX_PAIR_S;

void pair_init(VOCAL_SYS_S *sys_status);
void pair_detect(VOCAL_SYS_S *sys_status);

#ifdef __cplusplus
}
#endif

#endif /* _CC85XX_PAIR_H_ */