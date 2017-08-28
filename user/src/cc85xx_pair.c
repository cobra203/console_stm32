#include <stm32f0xx_gpio.h>

#include <cc85xx_pair.h>
#include <button.h>
#include <debug.h>
#include <vocal_sys.h>

static CC85XX_PAIR_S    cc85xx_pair;

typedef enum btn_type_e
{
    BTN_TYPE_MIC = 0,
    BTN_TYPE_SPK,  
} BTN_TYPE_E;

static void _pairing_process(CC85XX_PAIR_S *pair)
{
    VOCAL_SYS_S *vocal_sys = pair->vocal_sys;
    
    int i;
    for(i = 0; i < sizeof(pair->btn_pair)/sizeof(BUTTON_S); i++) {
        if(pair->btn_pair[i].state.avtice) {
            pair->btn_pair[i].state.avtice = 0;
            vocal_sys->sys_evt.req_pairing = STM_TRUE;
            DEBUG("[%d]:pairing\n", pair->btn_pair[i].type);
        }
    }
}

void pair_init(VOCAL_SYS_S *sys_status)
{ 
    int                 i;
    GPIO_InitTypeDef    init_struct = {0};

    cc85xx_pair.vocal_sys   = sys_status;

    init_struct.GPIO_Mode   = GPIO_Mode_IN;
    init_struct.GPIO_PuPd   = GPIO_PuPd_DOWN;
    init_struct.GPIO_Speed  = GPIO_Speed_Level_2;
    init_struct.GPIO_Pin    = GPIO_Pin_8 | GPIO_Pin_2;
    GPIO_Init(GPIOA, &init_struct);
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
    cc85xx_pair.process = _pairing_process;
    sys_status->pair    = &cc85xx_pair;
    
}

void pair_detect(VOCAL_SYS_S *sys_status)
{
    int         i;
    uint16_t    gpio_vol = GPIO_ReadInputData(GPIOA);
    
    cc85xx_pair.btn_pair[BTN_TYPE_MIC].state.press = (gpio_vol & GPIO_Pin_8) ? 0 : 1;
    cc85xx_pair.btn_pair[BTN_TYPE_SPK].state.press = (gpio_vol & GPIO_Pin_2) ? 0 : 1;

    for(i = 0; i < sizeof(cc85xx_pair.btn_pair)/sizeof(BUTTON_S); i++) {
        cc85xx_pair.btn_pair[i].check_active(&cc85xx_pair.btn_pair[i]);
    }
    cc85xx_pair.process(&cc85xx_pair);
}

