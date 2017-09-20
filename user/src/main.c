/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    05-December-2014
  * @brief   Main program body
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
#include <stm32f0xx_rcc.h>
#include <core_cm0.h>
#include <stm32f0xx_misc.h>

#include <debug.h>
#include <cc85xx_pair.h>

#include <stm32_timer.h>
#include <vocal_sys.h>


/** @addtogroup STM32F0xx_StdPeriph_Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

static void rcc_config(void)
{
    RCC_DeInit();

#if 0
    /* HSE = 24M */
    RCC_HSEConfig(RCC_HSE_ON);

    BIT_SET(RCC->CR, RCC_FLAG_HSERDY);

    if(RCC_WaitForHSEStartUp() != ERROR) { 
        /* PLL = HSE * RCC_PLLSource_PREDIV1 * 2 = 48M */
        RCC_PREDIV1Config(RCC_PREDIV1_Div1);
        RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_2);
        RCC_PLLCmd(ENABLE);
    }
#endif        
    //DEBUG("HSE Start Up Fail, Use HSI\n");
    RCC_HSEConfig(RCC_HSE_OFF);

    /* HSI = 8M */
    RCC_HSICmd(ENABLE);
    RCC_ClockSecuritySystemCmd(ENABLE);

    /* PLL = (HSI/2) * 12 = 48M */
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);
    RCC_PLLCmd(ENABLE);

    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    /* SYSCLK = PLLCLK = 48M */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    while(RCC_GetSYSCLKSource() != 0x08);

    /* AHB CLK(HCLK) = SYS CLK = 48M */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    /* APB CLK(PCLK) = HCLK = 48M */
    RCC_PCLKConfig(RCC_HCLK_Div1);
    
    /* Enable GPIO Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    
    /* Set Flash Latency */
    FLASH_PrefetchBufferCmd(ENABLE);
    FLASH_SetLatency(FLASH_Latency_1);

    
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
    //while(1);
    
    VOCAL_SYS_S vocal_sys = {0};

    
    /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f0xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f0xx.c file
     */ 
      
    /* Add your application code here
     */
    rcc_config();

    vocal_init(&vocal_sys);

    /* Infinite loop */
    while (1)
    {
        vocal_working(&vocal_sys);
    }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    DEBUG("ASSERT FAILED");
    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
