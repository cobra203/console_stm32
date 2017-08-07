#include <stm32_led.h>
#include <stm32_timer.h>
#include <stm32f0xx.h>

#define TASK_BUTT   (sizeof(uint16_t) * 2)
static void _led_id_transform(uint8_t id, uint16_t *pin)
{
    if(id < 4) {
        *pin = 0x1 << (10 + id);
    }
    else if(id < 6) {
        *pin = 0x1 << (id - 4);
    }
    else {
        *pin = 0x1 << id;
    } 
}

static void _led_bright(STM32_LED_S *led)
{
    GPIO_SetBits(led->port, led->pin);
    led->status = 1;
}

static void _led_dark(STM32_LED_S *led)
{
    GPIO_ResetBits(led->port, led->pin);
    led->status = 0;
}

static void _led_set_attr(STM32_LED_S *led, uint16_t dark_time, uint16_t bright_time)
{
    led->interval[0]    = dark_time;
    led->interval[1]    = bright_time;
}

static void _led_callback_func(void *args)
{
    STM32_LED_S *led = (STM32_LED_S *)args;

    if(!led->status) {
        led->bright(led);
        timer_set_reload(led->task, led->interval[0]);
    }
    else {
        led->dark(led);
        timer_set_reload(led->task, led->interval[1]);
    }
}

static void _led_doing(STM32_LED_S *led)
{
    if(led->task < TASK_BUTT) {
        timer_free(led->task);
        led->dark(led);
    }

    if(!led->interval[1]) {
        led->dark(led);
    }
    else if(!led->interval[0]) {
        led->bright(led);
    }
    else {
        while(timer_alloc(&led->task));
        timer_task(TMR_CYCLICITY, led->interval[0], led->interval[1], _led_callback_func, (void *)led);
    }
}

void stm32_led_init(STM32_LED_S *led, uint16_t id)
{
    GPIO_InitTypeDef    gpio_struct;

    _led_id_transform(id, &led->pin);
    led->port = (id < 6) ? GPIOA : GPIOB;
    
    gpio_struct.GPIO_Speed  = GPIO_Speed_Level_3;
    gpio_struct.GPIO_Mode   = GPIO_Mode_OUT;
    gpio_struct.GPIO_OType  = GPIO_OType_PP;
    gpio_struct.GPIO_Pin    = led->pin;
    GPIO_Init(led->port, &gpio_struct);

    led->set    = _led_set_attr;
    led->doing  = _led_doing;
    led->dark   = _led_dark;
    led->bright = _led_bright;

    led->task   = TASK_BUTT;
    led->status = 0;
}


