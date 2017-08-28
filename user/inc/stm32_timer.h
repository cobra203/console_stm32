#ifndef STM32_TIMER_H
#define STM32_TIMER_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>

typedef void (*TASK_F) (void *);

typedef enum timer_type_e
{
    TMR_ONCE,
    TMR_CYCLICITY,
} TIMER_TYPE_E;

#define TIMERS_NUM (sizeof(uint16_t) * 8)

typedef struct timer_obj_s
{
    uint16_t            active;
    uint16_t            touch;
    TIMER_TYPE_E        type[TIMERS_NUM];
    uint32_t            delay_reload[TIMERS_NUM];
    volatile uint32_t   delay_count[TIMERS_NUM];
    TASK_F              callback_func[TIMERS_NUM];
    void                *callback_args[TIMERS_NUM];
} TIMER_OBJ_S;

extern TIMER_OBJ_S basic_timer;

void timer_free(uint8_t *timer);
void timer_set_reload(uint8_t id, uint32_t reload);
void timer_itc(void);
void timer_init(void);
void timer_task(uint8_t *timer, TIMER_TYPE_E type, uint32_t delay, uint32_t load, TASK_F task, void *args);
void delay_ms(uint32_t time);

#ifdef __cplusplus
}
#endif

#endif /* STM32_TIMER_H */

