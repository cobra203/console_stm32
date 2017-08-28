#include <stm32_timer.h>
#include <vocal_common.h>

static TIMER_OBJ_S basic_timer;

static int timer_alloc(uint8_t *timer)
{
    uint8_t i;
    uint16_t active = basic_timer.active;

    for(i = 0; i < sizeof(active) * 8; i++, active >>= 1) {
        if(!(active & 0x1)) {
            BIT_SET(basic_timer.active, i);
            *timer = i;
            return 0;
        }
    }

    return -1;
}

static void _timer_touch_process(uint8_t idx)
{
    TASK_F      callback_func = 0;
    void        *callback_args = 0;

    if(!basic_timer.callback_func[idx]) {
        return;
    }
    
    switch(basic_timer.type[idx]) {
    case TMR_ONCE:
        callback_func   = basic_timer.callback_func[idx];
        callback_args   = basic_timer.callback_args[idx];
        timer_free(&idx);
        callback_func(callback_args);
        break;
    case TMR_CYCLICITY:

        BIT_CLR(basic_timer.touch, idx);
        basic_timer.delay_count[idx] = basic_timer.delay_reload[idx];
        basic_timer.callback_func[idx](basic_timer.callback_args[idx]);
        break;
    default:
        timer_free(&idx);
        break;
    }
}

void timer_free(uint8_t *timer)
{
    BIT_CLR(basic_timer.active, *timer);
    BIT_CLR(basic_timer.touch, *timer);
    basic_timer.delay_count[*timer] = 0;
    basic_timer.callback_func[*timer] = 0;
    *timer = TIMERS_NUM;
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
        if(active & 0x1) {
            if(basic_timer.delay_count[i]) {
                basic_timer.delay_count[i]--;
                if(!basic_timer.delay_count[i]) {
                    BIT_SET(basic_timer.touch, i);
                    _timer_touch_process(i);
                }
            }
        }
    }
}

void timer_init(void) /* 1ms timer */
{
    TIM_TimeBaseInitTypeDef tim_base_cfg    = {0};
    NVIC_InitTypeDef        nvic_cfg        = {0};
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    //__disable_irq();
#if 0

    NVIC_SetPriority(TIM2_IRQn, 2);
    NVIC_EnableIRQ(TIM2_IRQn);
#endif

#if 1

    nvic_cfg.NVIC_IRQChannelPriority    = 0;
    nvic_cfg.NVIC_IRQChannel            = TIM2_IRQn;
    nvic_cfg.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&nvic_cfg);
#endif

#if 1
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructInit(&tim_base_cfg);
    tim_base_cfg.TIM_Prescaler      = 1;
    tim_base_cfg.TIM_Period         = 0x1000;
#endif

#if 0
    tim_base_cfg.TIM_Prescaler      = 4800;
    tim_base_cfg.TIM_Period         = 10;
    tim_base_cfg.TIM_ClockDivision  = TIM_CKD_DIV1;
    tim_base_cfg.TIM_CounterMode    = TIM_CounterMode_Up;
#endif    
    TIM_TimeBaseInit(TIM2, &tim_base_cfg);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);    
    //__enable_irq();

    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    
}

void delay_ms(uint32_t time)   
{
    uint8_t timer;

    timer_task(&timer, TMR_ONCE, time, 0, 0, 0);
    
    while(basic_timer.delay_count[timer]);

    timer_free(&timer);
}

void timer_task(uint8_t *timer, TIMER_TYPE_E type, uint32_t delay, uint32_t load, TASK_F task, void *args)
{
    while(timer_alloc(timer));
    BIT_CLR(basic_timer.touch, *timer);

    basic_timer.type[*timer]            = type;
    basic_timer.callback_func[*timer]   = task;
    basic_timer.callback_args[*timer]   = args;
    basic_timer.delay_reload[*timer]    = load;
    basic_timer.delay_count[*timer]     = delay;
}
