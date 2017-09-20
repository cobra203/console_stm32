#include <stm32f0xx_gpio.h>

#include <cc85xx_pair.h>
#include <button.h>
#include <debug.h>
#include <vocal_sys.h>
#include <cc85xx_dev.h>
#include <vocal_common.h>

static CC85XX_PAIR_S    cc85xx_pair;

static void _pairing_process(CC85XX_PAIR_S *pair)
{
    VOCAL_SYS_S *vocal_sys = pair->vocal_sys;
    static int  focused_times[sizeof(pair->btn_pair)/sizeof(BUTTON_S)] = {0};
    
    int i;
    for(i = 0; i < sizeof(pair->btn_pair)/sizeof(BUTTON_S); i++) {
        if(pair->btn_pair[i].state.avtice) {
            DEBUG("effective=%d\n", pair->btn_pair[i].state.effective);
            if(ECT_FOCUSED == pair->btn_pair[i].state.effective) {
                focused_times[i]++;
                if(90 == focused_times[i]) {
                    vocal_sys->sys_evt.req_record_clean = STM_TRUE;
                    focused_times[i] = 0;
                }
            }
            else {
                focused_times[i] = 0;
            }
            vocal_sys->sys_evt.req_pairing = STM_TRUE;
        }
    }
}

static void _pairing_set_active(CC85XX_PAIR_S *pair, VOCAL_DEV_TYPE_E type)
{
    pair->btn_pair[type].state.avtice = STM_TRUE;
}

void pair_init(VOCAL_SYS_S *sys_status)
{ 
    int                 i;
    GPIO_InitTypeDef    init_struct = {0};

    cc85xx_pair.vocal_sys   = sys_status;

    init_struct.GPIO_Mode   = GPIO_Mode_IN;
    init_struct.GPIO_PuPd   = GPIO_PuPd_UP;
    init_struct.GPIO_Speed  = GPIO_Speed_Level_3;
    init_struct.GPIO_Pin    = PAIR_PIN_SPK | PAIR_PIN_MIC;
    GPIO_Init(PAIR_GPIO, &init_struct);
    
#if 0
    init_struct.GPIO_Mode   = GPIO_Mode_OUT;
    init_struct.GPIO_Speed  = GPIO_Speed_Level_2;
    init_struct.GPIO_Pin    = GPIO_Pin_8 | GPIO_Pin_2;
    GPIO_Init(GPIOB, &init_struct);
#endif

    for(i = 0; i < sizeof(cc85xx_pair.btn_pair)/sizeof(BUTTON_S); i++) {
        cc85xx_pair.btn_pair[i].check_active      = button_check_active;
        cc85xx_pair.btn_pair[i].type              = i;
        cc85xx_pair.btn_pair[i].interval.shack    = 3;
        cc85xx_pair.btn_pair[i].interval.pressed  = 20;
        cc85xx_pair.btn_pair[i].interval.focused  = 100;
    }
    cc85xx_pair.process     = _pairing_process;
    cc85xx_pair.set_active  = _pairing_set_active;
    sys_status->pair    = &cc85xx_pair;
    
}

void pair_itc(void)
{
    int16_t gpio_vol = GPIO_ReadInputData(PAIR_GPIO);

    cc85xx_pair.btn_pair[DEV_TYPE_SPK].state.press = (~gpio_vol) & PAIR_PIN_SPK ? 1 : 0;
    cc85xx_pair.btn_pair[DEV_TYPE_MIC].state.press = (~gpio_vol) & PAIR_PIN_MIC ? 1 : 0;
}

void pair_detect(void)
{
    int         i;

    for(i = 0; i < sizeof(cc85xx_pair.btn_pair)/sizeof(BUTTON_S); i++) {
        cc85xx_pair.btn_pair[i].check_active(&cc85xx_pair.btn_pair[i]);
    }
    cc85xx_pair.process(&cc85xx_pair);
}

