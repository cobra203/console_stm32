#include <vocal_led.h>
#include <stm32_led.h>
#include <debug.h>
static VOCAL_LED_S leds;

static void led_status_set(VOCAL_LED_S *led, VOCAL_DEV_TYPE_E type, uint8_t idx, LED_STATUS_E mode)
{
    STM32_LED_S *led_head = (type == DEV_TYPE_SPK) ? &led->spk_led[0] : &led->mic_led[0];

    switch(mode) {
    case LED_STATUS_CLOSED:
        led_head[idx].set(&led_head[idx], 1, 0);
        break;
    case LED_STATUS_CONNECT:
        led_head[idx].set(&led_head[idx], 0, 1);
        break;
    case LED_STATUS_PAIRING:
        led_head[idx].set(&led_head[idx], 100, 400);
        break;
    default:
        break;
    }

    led_head[idx].doing(&led_head[idx]);
}

void led_init(VOCAL_SYS_S *vocal_sys)
{
    int i = 0;
    
    leds.vocal_sys  = vocal_sys;
    vocal_sys->led  = &leds;
    leds.set        = led_status_set;

    for(i = 0; i < SPK_DEV_NUM; i++) {
        leds.spk_led[i].init = stm32_led_init;
        leds.spk_led[i].init(&leds.spk_led[i], i);
        leds.set(&leds, DEV_TYPE_SPK, i, LED_STATUS_CONNECT);
    }
    
    for(i = 0; i < MIC_DEV_NUM; i++) {
        leds.mic_led[i].init = stm32_led_init;
        leds.mic_led[i].init(&leds.mic_led[i], i + SPK_DEV_NUM);
        leds.set(&leds, DEV_TYPE_MIC, i, LED_STATUS_CONNECT);
    }
}

