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
    
    int i;
    for(i = 0; i < sizeof(pair->btn_pair)/sizeof(BUTTON_S); i++) {
        if(pair->btn_pair[i].state.avtice) {
            vocal_sys->sys_evt.req_pairing = STM_TRUE;
        }
    }
}

void pair_init(VOCAL_SYS_S *sys_status)
{ 
    int                 i;
    GPIO_InitTypeDef    init_struct = {0};
    EXTI_InitTypeDef    exti_cfg = {0};
    NVIC_InitTypeDef    nvic_cfg;

    cc85xx_pair.vocal_sys   = sys_status;

    init_struct.GPIO_Mode   = GPIO_Mode_IN;
    init_struct.GPIO_PuPd   = GPIO_PuPd_UP;
    init_struct.GPIO_Speed  = GPIO_Speed_Level_2;
    init_struct.GPIO_Pin    = GPIO_Pin_8 | GPIO_Pin_2;
    GPIO_Init(GPIOA, &init_struct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2);
    exti_cfg.EXTI_Line      = EXTI_Line2;
    exti_cfg.EXTI_Mode      = EXTI_Mode_Interrupt;
    exti_cfg.EXTI_Trigger   = EXTI_Trigger_Rising_Falling;
    exti_cfg.EXTI_LineCmd   = ENABLE;
    EXTI_Init(&exti_cfg);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
    exti_cfg.EXTI_Line      = EXTI_Line8;
    exti_cfg.EXTI_Mode      = EXTI_Mode_Interrupt;
    exti_cfg.EXTI_Trigger   = EXTI_Trigger_Rising_Falling;
    exti_cfg.EXTI_LineCmd   = ENABLE;
    EXTI_Init(&exti_cfg);
    
    nvic_cfg.NVIC_IRQChannelPriority    = 3;
    nvic_cfg.NVIC_IRQChannel            = EXTI2_3_IRQn;
    nvic_cfg.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&nvic_cfg);

    nvic_cfg.NVIC_IRQChannelPriority    = 3;
    nvic_cfg.NVIC_IRQChannel            = EXTI4_15_IRQn;
    nvic_cfg.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&nvic_cfg);
    
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

void pair_itc(void)
{
    int16_t gpio_vol = GPIO_ReadInputData(GPIOA);

    cc85xx_pair.btn_pair[DEV_TYPE_SPK].state.press = (~gpio_vol >> 2) & 0x1;
    cc85xx_pair.btn_pair[DEV_TYPE_MIC].state.press = (~gpio_vol >> 8) & 0x1;  
}

void pair_detect(void)
{
    int         i;

    for(i = 0; i < sizeof(cc85xx_pair.btn_pair)/sizeof(BUTTON_S); i++) {
        cc85xx_pair.btn_pair[i].check_active(&cc85xx_pair.btn_pair[i]);
    }
    cc85xx_pair.process(&cc85xx_pair);
}

