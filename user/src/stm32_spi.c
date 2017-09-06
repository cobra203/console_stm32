#include <stm32f0xx_syscfg.h>
#include <stm32f0xx_exti.h>
#include <stm32f0xx_gpio.h>
#include <stm32_spi.h>
#include <debug.h>

#define _WAIT_SPI_STAUTS_SET(spi, flag)     while(SPI_I2S_GetFlagStatus((spi), (flag)) == RESET)

#define WAIT_SEND_EN(spi)     _WAIT_SPI_STAUTS_SET((spi), SPI_I2S_FLAG_TXE)
#define WAIT_RECV_EN(spi)     _WAIT_SPI_STAUTS_SET((spi), SPI_I2S_FLAG_RXNE)

#define _WAIT_SPI_STAUTS_OR_CS_SET(spi, flag, cs_gpio, cs_pin) \
    while(GPIO_ReadInputDataBit((cs_gpio), (cs_pin)) == Bit_RESET && SPI_I2S_GetFlagStatus((spi), (flag)) == RESET)

#define SLAVE_WAIT_SEND_EN(spi, cg, cp) _WAIT_SPI_STAUTS_OR_CS_SET((spi), SPI_I2S_FLAG_TXE, (cg), (cp))
#define SLAVE_WAIT_RECV_EN(spi, cg, cp) _WAIT_SPI_STAUTS_OR_CS_SET((spi), SPI_I2S_FLAG_RXNE, (cg), (cp))

static SPI_SLAVE_ITC_S gs_spi_slave_itc;

#if 0
uint8_t *cur_send;
uint8_t *cur_recv;
uint8_t send_flag;
uint8_t recv_flag;

void spi_master_recv_itc(void)
{
    if(recv_flag == 1) {
        *cur_recv = SPI_ReceiveData8(SPI1);
    }
    else if(recv_flag == 0) {
        SPI_ReceiveData8(SPI1);
    }

    recv_flag = 2;
}


void spi_master_send_itc(void)
{
    if(send_flag == 1) {
        SPI_SendData8(SPI1, *cur_send);
    }
    else if(send_flag == 0) {
        SPI_SendData8(SPI1, 0xff);
    }

    send_flag = 2;
}
#endif

static void spi_master_transfer(SPI_S *spi, const void *send_data, uint8_t send_sz, void *recv_data, uint8_t recv_sz)
{
    uint8_t         send_len     = send_sz;
    uint8_t         recv_len     = recv_sz;
    uint8_t         size        = send_sz > recv_sz ? send_sz : recv_sz;
    const uint8_t   *sendbuf    = (const uint8_t *)send_data;
    uint8_t         *recvbuf    = (uint8_t *)recv_data;

#if 0
    cur_send    = (uint8_t *)send_data;
    cur_recv    = (uint8_t *)recv_data;

    while(size--) {
        if(send_len) {
            send_flag = 1;
            send_len--;
        }
        else {
            send_flag = 0;
        }
        if(recv_len) {
            recv_flag = 1;
            recv_len--;
        }
        else {
            recv_flag = 0;
        }

        SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, ENABLE);
            
        while(recv_flag != 2 || send_flag != 2);
        cur_send++;
        cur_recv++;
    }
#endif
#if 1
    while(size--) {
        WAIT_SEND_EN(spi->spi_id);
        send_len ? SPI_SendData8(spi->spi_id, sendbuf[send_sz - send_len--]) : SPI_SendData8(spi->spi_id, 0xff);
     
        WAIT_RECV_EN(spi->spi_id);
        recv_len ? recvbuf[recv_sz - recv_len--] = SPI_ReceiveData8(spi->spi_id) : SPI_ReceiveData8(spi->spi_id);
    }
#endif
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

static void spi2_st_init(SPI_S *spi)
{
    GPIO_InitTypeDef    gpio_cfg = {0};
    EXTI_InitTypeDef    exti_cfg = {0};
    NVIC_InitTypeDef    nvic_cfg;
    
    gpio_cfg.GPIO_Speed     = GPIO_Speed_Level_3;
    gpio_cfg.GPIO_OType     = GPIO_OType_PP;
    gpio_cfg.GPIO_Mode      = GPIO_Mode_AF;
    
    gpio_cfg.GPIO_Pin       = GPIO_Pin_5 | GPIO_Pin_7 | spi->pin_miso;
    GPIO_Init(GPIOA, &gpio_cfg);

    gpio_cfg.GPIO_Pin       = spi->cs_pin;
    gpio_cfg.GPIO_Mode      = GPIO_Mode_IN;
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

static void spi1_st_init(SPI_S *spi)
{
    GPIO_InitTypeDef    gpio_cfg = {0};
    
    gpio_cfg.GPIO_Speed     = GPIO_Speed_Level_3;
    gpio_cfg.GPIO_OType     = GPIO_OType_PP;
    gpio_cfg.GPIO_Mode      = GPIO_Mode_AF;
    
    gpio_cfg.GPIO_Pin       = GPIO_Pin_3 | GPIO_Pin_5 | spi->pin_miso;
    GPIO_Init(GPIOB, &gpio_cfg);

    gpio_cfg.GPIO_Pin       = spi->cs_pin;
    gpio_cfg.GPIO_Mode      = GPIO_Mode_OUT;
    GPIO_Init(GPIOA, &gpio_cfg);
    
    GPIO_WriteBit(GPIOA, spi->cs_pin, Bit_SET);  
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
    
    if(spi_id == SPI2) {
        spi->pin_miso           = GPIO_Pin_6;
        spi->cs_gpio            = GPIOA;
        spi->spi_gpio           = GPIOA;
        spi_struct.SPI_Mode     = SPI_Mode_Slave;
 
        spi2_st_init(spi);
        spi->transfer = spi_slave_transfer;
    }
    else if(spi_id == SPI1) {
        spi->pin_miso           = GPIO_Pin_4;
        spi->cs_gpio            = GPIOA;
        spi->spi_gpio           = GPIOB;
        spi_struct.SPI_Mode     = SPI_Mode_Master;

        spi1_st_init(spi);
        spi->transfer = spi_master_transfer;
    }

    SPI_I2S_DeInit(spi_id);
    
    spi_struct.SPI_Direction           = SPI_Direction_2Lines_FullDuplex;
    spi_struct.SPI_DataSize            = SPI_DataSize_8b;
    spi_struct.SPI_CPOL                = SPI_CPOL_Low;
    spi_struct.SPI_CPHA                = SPI_CPHA_1Edge;
    spi_struct.SPI_NSS                 = SPI_NSS_Soft;
    spi_struct.SPI_BaudRatePrescaler   = SPI_BaudRatePrescaler_4;
    spi_struct.SPI_FirstBit            = SPI_FirstBit_MSB;
    spi_struct.SPI_CRCPolynomial       = 0x7;

    spi->spi_id = spi_id;

#if 0
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Configure the SPI interrupt priority */
    NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

    SPI_Init(spi_id, &spi_struct);
    
    SPI_RxFIFOThresholdConfig(spi_id, SPI_RxFIFOThreshold_QF);
#if 0
    /* Enable the Rx buffer not empty interrupt */
    SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);
    /* Enable the SPI Error interrupt */
    SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_ERR, ENABLE);
#endif    
    SPI_Cmd(spi_id, ENABLE);
}
