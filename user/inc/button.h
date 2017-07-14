#ifndef _BUTTON_H_
#define _BUTTON_H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include <stm32f0xx.h>

typedef enum btn_effective_e
{
    ECT_LOOSE = 0,
    ECT_PRESSED,
    ECT_FOCUSED,
} BTN_EFFECTIVE_E;

typedef enum btn_shack_e
{
    SHACK_OLD_PRESS = 1,
    SHACK_OLD_NORMAL,
} BTN_SHACK_E;

typedef struct btn_state_s
{
    uint16_t    effective   :2;
    uint16_t    shack       :2;
    uint16_t    avtice      :1;
    uint16_t    press       :1;
    uint16_t    duration    :10;
} BTN_STATE_S;

typedef struct btn_interval_s
{
    uint8_t     shack;
    uint8_t     pressed;
    uint16_t    focused;
} BTN_INTERVAL_S;

typedef struct button_s
{
    BTN_STATE_S     state;
    uint8_t         type;
    BTN_INTERVAL_S  interval;
    int             (*is_press)(void *);
    int             (*check_active)(struct button_s *);
    int             (*process)(struct button_s *);
} BUTTON_S;

int button_check_active(BUTTON_S *button);

#ifdef __cplusplus
}
#endif

#endif /* _BUTTON_H_ */
