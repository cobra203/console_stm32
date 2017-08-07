#include <stm32f0xx_gpio.h>
#include <stm32_spi.h>

#define _WAIT_SPI_STAUTS_SET(spi, flag)     while(SPI_I2S_GetFlagStatus((spi), (flag)) == RESET)

#define WAIT_SEND_EN(spi)     _WAIT_SPI_STAUTS_SET((spi), SPI_I2S_FLAG_TXE)
#define WAIT_RECV_EN(spi)     _WAIT_SPI_STAUTS_SET((spi), SPI_I2S_FLAG_RXNE)


void spi_transfer(SPI_S *spi, const void *send_data, uint8_t send_len, void *recv_data, uint8_t recv_len)
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

void spi_init(SPI_S *spi, SPI_TypeDef *spi_id)
{
    SPI_InitTypeDef     spi_struct  = {0};
    GPIO_InitTypeDef    gpio_struct = {0};

    if(spi_id == SPI1) {
        spi_struct.SPI_Mode     = SPI_Mode_Slave;
 
        gpio_struct.GPIO_Speed  = GPIO_Speed_Level_3;
        gpio_struct.GPIO_Mode   = GPIO_Mode_AF;
        gpio_struct.GPIO_Pin    = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
        GPIO_Init(GPIOA, &gpio_struct);
    }
    else if(spi_id == SPI2) {
        spi->pin_miso           = GPIO_Pin_4;
        spi->cs_gpio            = GPIOA;
        spi->spi_gpio           = GPIOB;
        spi_struct.SPI_Mode     = SPI_Mode_Master;

        gpio_struct.GPIO_Speed  = GPIO_Speed_Level_3;
        gpio_struct.GPIO_Mode   = GPIO_Mode_AF;
        gpio_struct.GPIO_Pin    = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_Init(GPIOB, &gpio_struct);

        gpio_struct.GPIO_Pin    = GPIO_Pin_14 | GPIO_Pin_15;
        gpio_struct.GPIO_Mode   = GPIO_Mode_OUT;
        gpio_struct.GPIO_OType  = GPIO_OType_PP;
        gpio_struct.GPIO_PuPd   = GPIO_PuPd_UP;
        GPIO_Init(GPIOA, &gpio_struct);
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
