#include <stm32f0xx_syscfg.h>
#include <stm32f0xx_exti.h>
#include <stm32f0xx_gpio.h>
#include <stm32_spi.h>

#define _WAIT_SPI_STAUTS_SET(spi, flag)     while(SPI_I2S_GetFlagStatus((spi), (flag)) == RESET)

#define WAIT_SEND_EN(spi)     _WAIT_SPI_STAUTS_SET((spi), SPI_I2S_FLAG_TXE)
#define WAIT_RECV_EN(spi)     _WAIT_SPI_STAUTS_SET((spi), SPI_I2S_FLAG_RXNE)

#define _WAIT_SPI_STAUTS_OR_CS_SET(spi, flag, cs_gpio, cs_pin) \
    while(GPIO_ReadInputDataBit((cs_gpio), (cs_pin)) == Bit_RESET && SPI_I2S_GetFlagStatus((spi), (flag)) == RESET)

#define SLAVE_WAIT_SEND_EN(spi, cg, cp) _WAIT_SPI_STAUTS_OR_CS_SET((spi), SPI_I2S_FLAG_TXE, (cg), (cp))
#define SLAVE_WAIT_RECV_EN(spi, cg, cp) _WAIT_SPI_STAUTS_OR_CS_SET((spi), SPI_I2S_FLAG_RXNE, (cg), (cp))

static SPI_SLAVE_ITC_S gs_spi_slave_itc;

static void spi1_st_init(void)
{
    GPIO_InitTypeDef    gpio_cfg = {0};
    EXTI_InitTypeDef    exti_cfg = {0};
    NVIC_InitTypeDef    nvic_cfg;
    
    gpio_cfg.GPIO_Speed     = GPIO_Speed_Level_3;
    gpio_cfg.GPIO_Mode      = GPIO_Mode_AF;
    gpio_cfg.GPIO_Pin       = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio_cfg);

    gpio_cfg.GPIO_Pin       = GPIO_Pin_4;
    gpio_cfg.GPIO_Mode      = GPIO_Mode_IN;
    gpio_cfg.GPIO_PuPd      = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio_cfg);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);
    exti_cfg.EXTI_Line      = EXTI_Line4;
    exti_cfg.EXTI_Mode      = EXTI_Mode_Interrupt;
    exti_cfg.EXTI_Trigger   = EXTI_Trigger_Falling;
    exti_cfg.EXTI_LineCmd   = ENABLE;
    EXTI_Init(&exti_cfg);

    nvic_cfg.NVIC_IRQChannelPriority    = 3;
    nvic_cfg.NVIC_IRQChannel            = EXTI4_15_IRQn;
    nvic_cfg.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&nvic_cfg);
}

static void spi2_st_init(void)
{
    GPIO_InitTypeDef    gpio_cfg = {0};
    
    gpio_cfg.GPIO_Speed     = GPIO_Speed_Level_3;
    gpio_cfg.GPIO_Mode      = GPIO_Mode_AF;
    gpio_cfg.GPIO_Pin       = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOB, &gpio_cfg);

    gpio_cfg.GPIO_Pin       = GPIO_Pin_14 | GPIO_Pin_15;
    gpio_cfg.GPIO_Mode      = GPIO_Mode_OUT;
    gpio_cfg.GPIO_OType     = GPIO_OType_PP;
    gpio_cfg.GPIO_PuPd      = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio_cfg);
}

static void spi_master_transfer(SPI_S *spi, const void *send_data, uint8_t send_len, void *recv_data, uint8_t recv_len)
{
    uint8_t         send_sz     = send_len;
    uint8_t         recv_sz     = recv_len;
    uint8_t         size        = send_len > recv_len ? send_len : recv_len;
    const uint8_t   *sendbuf    = (const uint8_t *)send_data;
    uint8_t         *recvbuf    = (uint8_t *)recv_data;

    while(size--) {
        WAIT_SEND_EN(spi->spi_id);
        send_len ? SPI_SendData8(spi->spi_id, sendbuf[send_sz - send_len--]) : SPI_SendData8(spi->spi_id, 0xff);
     
        WAIT_RECV_EN(spi->spi_id);
        recv_len ? recvbuf[recv_sz - recv_len--] = SPI_ReceiveData8(spi->spi_id) : SPI_ReceiveData8(spi->spi_id);      
    }
}

static void spi_slave_transfer(SPI_S *spi, const void *send_data, uint8_t send_len, void *recv_data, uint8_t recv_len)
{
    int             i;
    uint8_t         size        = send_len > recv_len ? send_len : recv_len;
    const uint8_t   *sendbuf    = (const uint8_t *)send_data;
    uint8_t         *recvbuf    = (uint8_t *)recv_data;
    
    for(i = 0; i < size || !size; i++) {
        
        SLAVE_WAIT_RECV_EN(spi->spi_id, spi->cs_gpio, spi->cs_pin);
        if(GPIO_ReadInputDataBit(spi->cs_gpio, spi->cs_pin) != Bit_RESET) {
            break;
        }
        (recv_len > i) ? recvbuf[i] = SPI_ReceiveData8(spi->spi_id) : SPI_ReceiveData8(spi->spi_id);

        SLAVE_WAIT_SEND_EN(spi->spi_id, spi->cs_gpio, spi->cs_pin);
        if(GPIO_ReadInputDataBit(spi->cs_gpio, spi->cs_pin) != Bit_RESET) {
            break;
        }
        (send_len > i) ? SPI_SendData8(spi->spi_id, sendbuf[i]) : SPI_SendData8(spi->spi_id, 0);
    }
    
}

void spi_itc(void)
{
    gs_spi_slave_itc.itc(gs_spi_slave_itc.args);
    EXTI_ClearITPendingBit(EXTI_Line4);
}

void spi_init_slave_itc(SPI_S *spi, void (*callback)(void *), void *args)
{
    gs_spi_slave_itc.itc    = callback;
    gs_spi_slave_itc.args   = args;
}

void spi_init(SPI_S *spi, SPI_TypeDef *spi_id)
{
    SPI_InitTypeDef     spi_struct  = {0};
    
    if(spi_id == SPI1) {
        spi->pin_miso     = GPIO_Pin_6;
        spi->cs_gpio            = GPIOA;
        spi->spi_gpio           = GPIOA;
        spi_struct.SPI_Mode     = SPI_Mode_Slave;
 
        spi1_st_init();
        spi->transfer = spi_slave_transfer;
    }
    else if(spi_id == SPI2) {
        spi->pin_miso           = GPIO_Pin_4;
        spi->cs_gpio            = GPIOA;
        spi->spi_gpio           = GPIOB;
        spi_struct.SPI_Mode     = SPI_Mode_Master;

        spi2_st_init();
        spi->transfer = spi_master_transfer;
    }

    spi_struct.SPI_Direction           = SPI_Direction_2Lines_FullDuplex;
    spi_struct.SPI_DataSize            = SPI_DataSize_8b;
    spi_struct.SPI_CPOL                = SPI_CPOL_Low;
    spi_struct.SPI_CPHA                = SPI_CPHA_1Edge;
    spi_struct.SPI_NSS                 = SPI_NSS_Soft;
    spi_struct.SPI_BaudRatePrescaler   = SPI_BaudRatePrescaler_256;
    spi_struct.SPI_FirstBit            = SPI_FirstBit_MSB;
    spi_struct.SPI_CRCPolynomial       = 0x7;

    spi->spi_id = spi_id;

    SPI_Init(spi_id, &spi_struct);
}
