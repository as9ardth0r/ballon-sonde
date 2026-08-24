#ifndef SPI1_BUS_H
#define SPI1_BUS_H

#include <stdint.h>

void spi1_init(void);
uint8_t spi1_transfer(uint8_t byte);

#endif /* SPI1_BUS_H */
