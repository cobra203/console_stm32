/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    05-December-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include <stm32f0xx_exti.h>
#include <debug.h>
#include <stm32_timer.h>
#include <stm32_spi.h>
#include <cc85xx_pair.h>
#include <stm32f0xx_spi.h>
#include <vocal_common.h>

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    GPIO_InitTypeDef    gpio_struct;

    gpio_struct.GPIO_Speed  = GPIO_Speed_Level_3;
    gpio_struct.GPIO_Mode   = GPIO_Mode_OUT;
    gpio_struct.GPIO_PuPd   = GPIO_PuPd_UP;
    gpio_struct.GPIO_OType  = GPIO_OType_OD;
    gpio_struct.GPIO_Pin    = LED_PIN_MIC4;

    GPIO_Init(LED_GPIO_MIC4, &gpio_struct);
    GPIO_ResetBits(LED_GPIO_MIC4, gpio_struct.GPIO_Pin);
#if 0
    __disable_irq();
    NVIC_SystemReset();
#endif
    while (1)
    {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    //DEBUG("ss\n");
    timer_itc();
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

void TIM2_IRQHandler(void)
{
#if 1
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        timer_itc();
    }
#endif
}

void EXTI2_3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2) != RESET) {
        pair_itc();
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI4_15_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET) {                        
        //__disable_irq();
        //GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_RESET);
        spi_itc();
        //__enable_irq();
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
#if 0
    else if(EXTI_GetITStatus(EXTI_Line5) != RESET) {
        if(Bit_SET == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)) {

        }
        else {

        }
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
#endif
    else if(EXTI_GetITStatus(EXTI_Line8) != RESET) {
        pair_itc();
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
}
#if 0
void SPI1_IRQHandler(void)
{
    if(SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE) == SET) {
        spi_master_send_itc();
        SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, DISABLE);
    }

    if(SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_RXNE) == SET) {
        spi_master_recv_itc();
    }

    if(SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_OVR) == SET) {
        SPI_ReceiveData8(SPI1);
        SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_OVR);
    }
}
#endif
void SPI2_IRQHandler(void)
{

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
