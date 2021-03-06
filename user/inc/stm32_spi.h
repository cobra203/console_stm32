#ifndef _STM32_SPI_H_
#define _STM32_SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>
#include <stm32f0xx_spi.h>

typedef struct spi_slave_itc_s
{
    void *args;
    void (*itc) (void *);
} SPI_SLAVE_ITC_S;

typedef struct spi_s
{
    uint8_t         dev;
    SPI_TypeDef     *spi_id;
    GPIO_TypeDef    *gpio_cs;
    GPIO_TypeDef    *gpio_spi;
    uint16_t        pin_cs;
    uint16_t        pin_miso;
    uint16_t        pin_mosi;
    uint16_t        pin_sck;

    void (*init_slave_itc)  (struct spi_s *, void (*)(void *), void *);
    void (*init)            (struct spi_s *, SPI_TypeDef *);
    void (*set_enable)      (struct spi_s *, uint8_t);
    int  (*transfer)        (struct spi_s *, const void *, uint8_t, void *, uint8_t);
}SPI_S;


extern int tmpdebug;

void spi_itc(void);
void spi_init_slave_itc(SPI_S *spi, void (*callback)(void *), void *args);
void spi_init(SPI_S *spi, SPI_TypeDef *spi_id);
#if 0
void spi_master_recv_itc(void);
void spi_master_send_itc(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _STM32_SPI_H_ */

