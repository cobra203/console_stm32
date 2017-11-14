#include <stm32_timer.h>
#include <vocal_common.h>
#include <stm32f0xx_tim.h>
#include <debug.h>

static TIMER_OBJ_S basic_timer;

static int timer_alloc(uint8_t *timer)
{
    uint8_t i;
    uint16_t active = basic_timer.active;

    for(i = 0; i < TIMERS_NUM; i++, active >>= 1) {
        if(!(active & 0x1)) {
            *timer = i;
            return 0;
        }
    }

    return -1;
}

static void _timer_touch_process(uint8_t idx)
{
    TASK_F  callback_func   = basic_timer.callback_func[idx];
    void    *callback_args  = basic_timer.callback_args[idx];
  
    switch(basic_timer.type[idx]) {
    case TMR_ONCE:
        timer_free(&idx);
        break;
    case TMR_CYCLICITY:
        BIT_CLR(basic_timer.touch, idx);
        basic_timer.delay_count[idx] = basic_timer.delay_reload[idx];
        break;
    default:
        timer_free(&idx);
        return;
    }

    if(callback_func) {
        callback_func(callback_args);
    }
}

void timer_free(uint8_t *timer)
{
    if(*timer < TIMERS_NUM) {
        BIT_CLR(basic_timer.active, *timer);
        BIT_CLR(basic_timer.touch, *timer);
        basic_timer.delay_count[*timer] = 0;
        basic_timer.callback_func[*timer] = 0;
        *timer = TIMERS_NUM;
    }
}

void timer_set_reload(uint8_t id, uint32_t reload)
{
    basic_timer.delay_reload[id] = reload;
}

void timer_itc(void)
{
    uint8_t     i;
    uint16_t    active = basic_timer.active;

    for(i = 0; i < TIMERS_NUM; i++, active >>= 1) {
        if(active & 0x1 && basic_timer.delay_count[i]) {
            basic_timer.delay_count[i]--;
            if(!basic_timer.delay_count[i]) {
                BIT_SET(basic_timer.touch, i);
            }
        }
    }
}

void timer_init(void) /* 1ms timer */
{
#if 0
    TIM_TimeBaseInitTypeDef tim_base_cfg    = {0};
    NVIC_InitTypeDef        nvic_cfg        = {0};
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    nvic_cfg.NVIC_IRQChannelPriority    = 0;
    nvic_cfg.NVIC_IRQChannel            = TIM2_IRQn;
    nvic_cfg.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&nvic_cfg);
    
    tim_base_cfg.TIM_Prescaler      = 4800 - 1;
    tim_base_cfg.TIM_Period         = 10 - 1;
    tim_base_cfg.TIM_ClockDivision  = TIM_CKD_DIV1;
    tim_base_cfg.TIM_CounterMode    = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim_base_cfg);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);    
#endif
#if 1
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    SysTick_Config(48000 - 1);
#endif    
}

void delay_ms(uint32_t time)   
{
    uint8_t timer;

    timer_task(&timer, TMR_ONCE, time, 0, 0, 0);
    
    while(!BIT_ISSET(basic_timer.touch, timer));
    timer_free(&timer);
}

void timer_task(uint8_t *timer, TIMER_TYPE_E type, uint32_t delay, uint32_t load, TASK_F task, void *args)
{
    while(timer_alloc(timer));

    basic_timer.type[*timer]            = type;
    basic_timer.callback_func[*timer]   = task;
    basic_timer.callback_args[*timer]   = args;
    basic_timer.delay_reload[*timer]    = load;
    basic_timer.delay_count[*timer]     = delay ? delay : 1;
    
    BIT_SET(basic_timer.active, *timer);
}

void timer_task_process(void)
{
    uint8_t     i;
    uint16_t    touch = basic_timer.touch;

    for(i = 0; i < TIMERS_NUM; i++, touch >>= 1) {
        if(touch & 0x1) {
            _timer_touch_process(i);
        }
    }
}
