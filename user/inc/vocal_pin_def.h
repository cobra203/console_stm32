#ifndef VOCAL_PIN_DEF_H
#define VOCAL_PIN_DEF_H

#ifdef __cplusplus
 extern "C" {
#endif

#define VER_RELEASE         0
#define VER_DEBUG_DEMO      1
#define VER_DEBUG_ST_LINK   2

#define VERSION VER_DEBUG_DEMO

#if (VERSION == VER_RELEASE)

#define MIC_CS_GPIO         GPIOB
#define MIC_CS_PIN          GPIO_Pin_1

#elif (VERSION == VER_DEBUG_DEMO) 
#define MIC_CS_GPIO         GPIOA
#define MIC_CS_PIN          GPIO_Pin_14

#elif (VERSION == VER_DEBUG_ST_LINK)
#define MIC_CS_GPIO         GPIOA
#define MIC_CS_PIN          GPIO_Pin_10

#endif

#define SPK_CS_GPIO         GPIOA
#define SPK_CS_PIN          GPIO_Pin_15

#define TI_GPIO_SPI         GPIOB
#define TI_PIN_SCK          GPIO_Pin_3
#define TI_PIN_MISO         GPIO_Pin_4
#define TI_PIN_MOSI         GPIO_Pin_5

#define MCP_CS_GPIO         GPIOA           /* fixed */
#define MCP_CS_PIN          GPIO_Pin_4      /* fixed */

#define MCP_GPIO_SPI        GPIOA
#define MCP_PIN_SCK         GPIO_Pin_5
#define MCP_PIN_MISO        GPIO_Pin_6
#define MCP_PIN_MOSI        GPIO_Pin_7

#define PAIR_PIN_SPK        GPIO_Pin_2      /* fixed */
#define PAIR_PIN_MIC        GPIO_Pin_8      /* fixed */
#define PAIR_GPIO           GPIOA

#define LED_PIN_SPK1        GPIO_Pin_6
#define LED_PIN_SPK2        GPIO_Pin_7
#define LED_PIN_SPK3        GPIO_Pin_0
#define LED_PIN_SPK4        GPIO_Pin_1

#if (VERSION == VER_RELEASE)
#define LED_PIN_MIC1        GPIO_Pin_12
#define LED_PIN_MIC2        GPIO_Pin_11
#define LED_PIN_MIC3        GPIO_Pin_10
#define LED_PIN_MIC4        GPIO_Pin_9

#elif (VERSION == VER_DEBUG_DEMO)
#define LED_PIN_MIC1        GPIO_Pin_13
#define LED_PIN_MIC2        GPIO_Pin_12
#define LED_PIN_MIC3        GPIO_Pin_11
#define LED_PIN_MIC4        GPIO_Pin_10

#elif (VERSION == VER_DEBUG_ST_LINK)
#define LED_PIN_MIC1        GPIO_Pin_9
#define LED_PIN_MIC2        GPIO_Pin_12
#define LED_PIN_MIC3        GPIO_Pin_11
#define LED_PIN_MIC4        LED_PIN_MIC3

#endif

#define LED_GPIO_SPK1       GPIOB
#define LED_GPIO_SPK2       GPIOB
#define LED_GPIO_SPK3       GPIOA
#define LED_GPIO_SPK4       GPIOA

#define LED_GPIO_MIC1       GPIOA
#define LED_GPIO_MIC2       GPIOA
#define LED_GPIO_MIC3       GPIOA
#define LED_GPIO_MIC4       GPIOA

#ifdef __cplusplus
}
#endif

#endif /* VOCAL_PIN_DEF_H */

