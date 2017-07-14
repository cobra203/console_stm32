#ifndef _STM32_SPI_H_
#define _STM32_SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>
#include <stm32f0xx_spi.h>

typedef struct spi_s
{
    uint8_t     dev;
    SPI_TypeDef     *spi_id;
    GPIO_TypeDef    *cs_gpio;
    GPIO_TypeDef    *spi_gpio;
    uint16_t        cs_pin;
    uint16_t        pin_miso;
    
    void (*init)(struct spi_s *, SPI_TypeDef *);
    void (*set_enable)(struct spi_s *, uint8_t);
    void (*transfer)(struct spi_s *, const void *, uint8_t, void *, uint8_t);
}SPI_S;

void spi_init(SPI_S *spi, SPI_TypeDef *spi_id);
void spi_transfer(SPI_S *spi, const void *send_data, uint8_t send_len, void *recv_data, uint8_t recv_len);

#ifdef __cplusplus
}
#endif

#endif /* _STM32_SPI_H_ */

