/**************************************************
 *
 * Part one of the system initialization code, contains low-level
 * initialization, plain thumb variant.
 *
 * Copyright 2008 IAR Systems. All rights reserved.
 *
 * $Revision: 65537 $
 *
 **************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol __iar_program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY, which
; is where to find the SP start value.
; If vector table is not located at address 0, the user has to initialize the  NVIC vector
; table register (VTOR) before using interrupts.
;
; Cortex-M version
;

        MODULE  ?vector_table

        AAPCS INTERWORK, VFP_COMPATIBLE, RWPI_COMPATIBLE
        PRESERVE8


        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        PUBLIC  __vector_table

        DATA

__iar_init$$done:               ; The vector table is not needed
                                ; until after copy initialization is done

__vector_table
        DCD     sfe(CSTACK)
        DCD     __iar_program_start

        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     MemManage_Handler
        DCD     BusFault_Handler
        DCD     UsageFault_Handler
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler
        DCD     DebugMon_Handler
        DCD     0
        DCD     PendSV_Handler
        DCD     SysTick_Handler

        ; External Interrupts
        DCD     WWDG_IRQHandler                ; Window Watchdog
        DCD     PVD_IRQHandler                 ; PVD through EXTI Line detect
        DCD     RTC_IRQHandler                 ; RTC through EXTI Line
        DCD     FLASH_IRQHandler               ; FLASH
        DCD     RCC_IRQHandler                 ; RCC
        DCD     EXTI0_1_IRQHandler             ; EXTI Line 0 and 1
        DCD     EXTI2_3_IRQHandler             ; EXTI Line 2 and 3
        DCD     EXTI4_15_IRQHandler            ; EXTI Line 4 to 15
        DCD     TS_IRQHandler                  ; TS
        DCD     DMA1_Channel1_IRQHandler       ; DMA1 Channel 1
        DCD     DMA1_Channel2_3_IRQHandler     ; DMA1 Channel 2 and Channel 3
        DCD     DMA1_Channel4_5_IRQHandler     ; DMA1 Channel 4 and Channel 5
        DCD     ADC1_COMP_IRQHandler           ; ADC1, COMP1 and COMP2 
        DCD     TIM1_BRK_UP_TRG_COM_IRQHandler ; TIM1 Break, Update, Trigger and Commutation
        DCD     TIM1_CC_IRQHandler             ; TIM1 Capture Compare
        DCD     TIM2_IRQHandler                ; TIM2
        DCD     TIM3_IRQHandler                ; TIM3
        DCD     TIM6_DAC_IRQHandler            ; TIM6 and DAC
        DCD     0                              ; Reserved
        DCD     TIM14_IRQHandler               ; TIM14
        DCD     TIM15_IRQHandler               ; TIM15
        DCD     TIM16_IRQHandler               ; TIM16
        DCD     TIM17_IRQHandler               ; TIM17
        DCD     I2C1_IRQHandler                ; I2C1
        DCD     I2C2_IRQHandler                ; I2C2
        DCD     SPI1_IRQHandler                ; SPI1
        DCD     SPI2_IRQHandler                ; SPI2
        DCD     USART1_IRQHandler              ; USART1
        DCD     USART2_IRQHandler              ; USART2
        DCD     0                              ; Reserved
        DCD     CEC_IRQHandler                 ; CEC
        DCD     0                              ; Reserved
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;

        PUBWEAK NMI_Handler
        PUBWEAK HardFault_Handler
        PUBWEAK MemManage_Handler
        PUBWEAK BusFault_Handler
        PUBWEAK UsageFault_Handler
        PUBWEAK SVC_Handler
        PUBWEAK DebugMon_Handler
        PUBWEAK PendSV_Handler
        PUBWEAK SysTick_Handler
        PUBWEAK WWDG_IRQHandler                
        PUBWEAK PVD_IRQHandler                 
        PUBWEAK RTC_IRQHandler                 
        PUBWEAK FLASH_IRQHandler               
        PUBWEAK RCC_IRQHandler                 
        PUBWEAK EXTI0_1_IRQHandler             
        PUBWEAK EXTI2_3_IRQHandler             
        PUBWEAK EXTI4_15_IRQHandler            
        PUBWEAK TS_IRQHandler                  
        PUBWEAK DMA1_Channel1_IRQHandler       
        PUBWEAK DMA1_Channel2_3_IRQHandler     
        PUBWEAK DMA1_Channel4_5_IRQHandler     
        PUBWEAK ADC1_COMP_IRQHandler           
        PUBWEAK TIM1_BRK_UP_TRG_COM_IRQHandler 
        PUBWEAK TIM1_CC_IRQHandler             
        PUBWEAK TIM2_IRQHandler                
        PUBWEAK TIM3_IRQHandler                
        PUBWEAK TIM6_DAC_IRQHandler                                        
        PUBWEAK TIM14_IRQHandler               
        PUBWEAK TIM15_IRQHandler               
        PUBWEAK TIM16_IRQHandler               
        PUBWEAK TIM17_IRQHandler               
        PUBWEAK I2C1_IRQHandler                
        PUBWEAK I2C2_IRQHandler                
        PUBWEAK SPI1_IRQHandler                
        PUBWEAK SPI2_IRQHandler                
        PUBWEAK USART1_IRQHandler              
        PUBWEAK USART2_IRQHandler              
                            
CEC_IRQHandler                 
        SECTION .text:CODE:REORDER:NOROOT(1)
        THUMB

NMI_Handler
HardFault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
SVC_Handler
DebugMon_Handler
PendSV_Handler
SysTick_Handler
WWDG_IRQHandler               
PVD_IRQHandler                
RTC_IRQHandler                
FLASH_IRQHandler              
RCC_IRQHandler                
EXTI0_1_IRQHandler            
EXTI2_3_IRQHandler            
EXTI4_15_IRQHandler           
TS_IRQHandler                 
DMA1_Channel1_IRQHandler      
DMA1_Channel2_3_IRQHandler    
DMA1_Channel4_5_IRQHandler    
ADC1_COMP_IRQHandler          
TIM1_BRK_UP_TRG_COM_IRQHandler
TIM1_CC_IRQHandler            
TIM2_IRQHandler               
TIM3_IRQHandler               
TIM6_DAC_IRQHandler           
TIM14_IRQHandler              
TIM15_IRQHandler              
TIM16_IRQHandler              
TIM17_IRQHandler              
I2C1_IRQHandler               
I2C2_IRQHandler               
SPI1_IRQHandler               
SPI2_IRQHandler               
USART1_IRQHandler             
USART2_IRQHandler             
Default_Handler
__default_handler
        CALL_GRAPH_ROOT __default_handler, "interrupt"
        NOCALL __default_handler
        B __default_handler

        END
