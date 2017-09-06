#ifndef _VOCAL_LED_H_
#define _VOCAL_LED_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>
#include <vocal_sys.h>
#include <stm32_led.h>

typedef enum led_status_e
{
    LED_STATUS_CLOSED,
    LED_STATUS_CONNECT,
    LED_STATUS_PAIRING,
} LED_STATUS_E;

typedef struct vocal_led_s
{
    VOCAL_SYS_S     *vocal_sys;
    STM32_LED_S     spk_led[SPK_DEV_NUM];
    STM32_LED_S     mic_led[MIC_DEV_NUM];
    void            (*set)      (struct vocal_led_s *, VOCAL_DEV_TYPE_E, uint8_t, LED_STATUS_E);
} VOCAL_LED_S;

void led_init(VOCAL_SYS_S *vocal_sys);

#ifdef __cplusplus
}
#endif

#endif /* _VOCAL_LED_H_ */

