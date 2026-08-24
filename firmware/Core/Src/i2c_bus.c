#include "i2c_bus.h"
#include "stm32l0xx.h"

#define I2C_TIMEOUT_LOOPS 100000U

static bool wait_flag_set(volatile uint32_t *reg, uint32_t mask, uint32_t loops) {
    while ((*reg & mask) == 0) { if (--loops == 0) return false; }
    return true;
}

void i2c1_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* PB6=SCL, PB7=SDA, AF1, open-drain, pull-up interne */
    GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOB->MODER |= (2UL << GPIO_MODER_MODE6_Pos) | (2UL << GPIO_MODER_MODE7_Pos);
    GPIOB->OTYPER |= GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7;
    GPIOB->PUPDR |= (1UL << GPIO_PUPDR_PUPD6_Pos) | (1UL << GPIO_PUPDR_PUPD7_Pos);
    GPIOB->AFR[0] |= (1UL << GPIO_AFRL_AFSEL6_Pos) | (1UL << GPIO_AFRL_AFSEL7_Pos);

    I2C1->CR1 &= ~I2C_CR1_PE;
    /* TIMINGR pour 100 kHz avec I2CCLK = HSI16 (16 MHz) : valeur de
     * référence standard ST (AN4235 / exemples CubeMX) pour cette
     * configuration exacte, pas dérivée à la main. */
    I2C1->TIMINGR = 0x10420F13UL;
    I2C1->CR1 |= I2C_CR1_PE;
}

static bool do_write(uint8_t addr7, const uint8_t *data, size_t len, bool autoend) {
    uint32_t cr2 = ((uint32_t)addr7 << 1) | ((uint32_t)len << I2C_CR2_NBYTES_Pos)
                 | I2C_CR2_START;
    if (autoend) cr2 |= I2C_CR2_AUTOEND;
    I2C1->CR2 = cr2;

    for (size_t i = 0; i < len; i++) {
        if (!wait_flag_set(&I2C1->ISR, I2C_ISR_TXIS, I2C_TIMEOUT_LOOPS)) return false;
        I2C1->TXDR = data[i];
    }

    if (autoend) {
        if (!wait_flag_set(&I2C1->ISR, I2C_ISR_STOPF, I2C_TIMEOUT_LOOPS)) return false;
        I2C1->ICR = I2C_ICR_STOPCF;
    } else {
        if (!wait_flag_set(&I2C1->ISR, I2C_ISR_TC, I2C_TIMEOUT_LOOPS)) return false;
    }
    return true;
}

static bool do_read(uint8_t addr7, uint8_t *data, size_t len) {
    uint32_t cr2 = ((uint32_t)addr7 << 1) | ((uint32_t)len << I2C_CR2_NBYTES_Pos)
                 | I2C_CR2_START | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN;
    I2C1->CR2 = cr2;

    for (size_t i = 0; i < len; i++) {
        if (!wait_flag_set(&I2C1->ISR, I2C_ISR_RXNE, I2C_TIMEOUT_LOOPS)) return false;
        data[i] = (uint8_t)I2C1->RXDR;
    }

    if (!wait_flag_set(&I2C1->ISR, I2C_ISR_STOPF, I2C_TIMEOUT_LOOPS)) return false;
    I2C1->ICR = I2C_ICR_STOPCF;
    return true;
}

bool i2c1_write(uint8_t addr7, const uint8_t *data, size_t len) {
    return do_write(addr7, data, len, true);
}

bool i2c1_write_read(uint8_t addr7, const uint8_t *reg, size_t reg_len,
                      uint8_t *data, size_t data_len) {
    if (!do_write(addr7, reg, reg_len, false)) return false; /* pas de STOP : repeated start */
    return do_read(addr7, data, data_len);
}
