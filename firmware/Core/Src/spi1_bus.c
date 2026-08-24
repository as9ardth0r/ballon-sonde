#include "spi1_bus.h"
#include "stm32l0xx.h"

void spi1_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* PA5=SCK, PA6=MISO, PA7=MOSI, AF0 = SPI1 */
    GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOA->MODER |= (2UL << GPIO_MODER_MODE5_Pos) | (2UL << GPIO_MODER_MODE6_Pos)
                   | (2UL << GPIO_MODER_MODE7_Pos);
    /* AFR AF0 = 0, pas besoin de positionner de bits */

    /* CPOL=0, CPHA=0 (mode 0, standard SX1276), maître, baud = fPCLK/8,
     * 8 bits, NSS logiciel (géré en GPIO séparé, voir lora_sx1276.c) */
    SPI1->CR1 = SPI_CR1_MSTR | (2UL << SPI_CR1_BR_Pos) | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR2 = 0;
    SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t spi1_transfer(uint8_t byte) {
    while ((SPI1->SR & SPI_SR_TXE) == 0) { }
    *(volatile uint8_t *)&SPI1->DR = byte;
    while ((SPI1->SR & SPI_SR_RXNE) == 0) { }
    return (uint8_t)SPI1->DR;
}
