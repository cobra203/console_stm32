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

typedef struct timer_obj_s
{
    uint16_t            active;
    uint16_t            touch;
    TIMER_TYPE_E        type;
    uint32_t            delay_reload[sizeof(active) * 8];
    volatile uint32_t   delay_count[sizeof(active) * 8];
    TASK_F              callback_func[sizeof(active) * 8];
    void                *callback_args[sizeof(active) * 8];
} TIMER_OBJ_S;

extern TIMER_OBJ_S basic_timer;

int  timer_alloc(uint8_t *timer);
void timer_free(uint8_t timer);
void timer_set_reload(uint8_t id, uint32_t reload);
void timer_itc(void);
void timer_init(void);
void timer_touch_process(void);
void timer_task(TIMER_TYPE_E type, uint32_t delay, uint32_t load, TASK_F task, void *args);
void delay_ms(uint32_t time);

#ifdef __cplusplus
}
#endif

#endif /* STM32_TIMER_H */

