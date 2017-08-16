#include <stm32_timer.h>
#include <vocal_common.h>

static TIMER_OBJ_S basic_timer;

int timer_alloc(uint8_t *timer)
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

void timer_free(uint8_t timer)
{
    BIT_CLR(basic_timer.active, timer);
    BIT_CLR(basic_timer.touch, timer);
    basic_timer.delay_count[timer] = 0;
    basic_timer.callback_func[timer] = 0;   
}

void timer_set_reload(uint8_t id, uint32_t reload)
{
    basic_timer.delay_reload[id] = reload;
}

void timer_itc(void)
{
    uint8_t     i;
    uint16_t    active = basic_timer.active;
    
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
    else {
        return;
    }

    for(i = 0; i < sizeof(active) * 8; i++, active >>= 1) {
        if(active & 0x1) {
            if(basic_timer.delay_count[i]) {
                basic_timer.delay_count[i]--;
                if(!basic_timer.delay_count[i]) {
                    BIT_SET(basic_timer.touch, i);
                }
            }
        }
    }
}

void timer_init(void) /* 1ms timer */
{
    TIM_TimeBaseInitTypeDef tim_base_cfg;
    NVIC_InitTypeDef        nvic_cfg;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    tim_base_cfg.TIM_Prescaler      = 4800;
    tim_base_cfg.TIM_Period         = 10;
    tim_base_cfg.TIM_ClockDivision  = TIM_CKD_DIV1;
    tim_base_cfg.TIM_CounterMode    = TIM_CounterMode_Down;
    
    TIM_TimeBaseInit(TIM2, &tim_base_cfg);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    nvic_cfg.NVIC_IRQChannelPriority    = 2;
    nvic_cfg.NVIC_IRQChannel            = TIM2_IRQn;
    nvic_cfg.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&nvic_cfg);
    
    TIM_Cmd(TIM2, ENABLE);
}

void timer_touch_process(void)
{
    uint8_t     i;
    uint16_t    avtice = basic_timer.active;
    TASK_F      callback = 0;
    void        *callback_args = 0;

    for(i = 0; i < sizeof(avtice) * 8; i++, avtice >>= 1) {
        if(avtice & 0x1 && BIT_ISSET(basic_timer.touch, i)) {
            switch(basic_timer.type) {
            case TMR_ONCE:
                callback = basic_timer.callback_func[i];
                callback_args = basic_timer.callback_args[i];
                timer_free(i);
                callback(callback_args);
                break;
            case TMR_CYCLICITY:
                BIT_CLR(basic_timer.touch, i);
                basic_timer.delay_count[i] = basic_timer.delay_reload[i];
                basic_timer.callback_func[i](basic_timer.callback_args[i]);
                break;
            default:
                timer_free(i);
                break;
            }
        }
    }
}

void delay_ms(uint32_t time)   
{
    uint8_t timer;
    
    while(timer_alloc(&timer));
    basic_timer.delay_count[timer] = time;

    while(basic_timer.delay_count[timer]);

    timer_free(timer);
}

void timer_task(TIMER_TYPE_E type, uint32_t delay, uint32_t load, TASK_F task, void *args)
{
    uint8_t timer;

    while(timer_alloc(&timer));
    BIT_CLR(basic_timer.touch, timer);
    
    basic_timer.callback_func[timer]    = task;
    basic_timer.callback_args[timer]    = args;
    basic_timer.delay_reload[timer]     = load;
    basic_timer.delay_count[timer]      = delay;
}
