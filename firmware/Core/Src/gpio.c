#include "gpio.h"
#include "stm32l0xx.h"

static GPIO_TypeDef *port_from_index(uint8_t index) {
    return (GPIO_TypeDef *)(GPIOA_BASE + (index * 0x0400UL));
}

static void enable_port_clock(uint8_t index) {
    RCC->IOPENR |= (1UL << index);
}

void gpio_init_output(uint8_t port_pin) {
    uint8_t port_index = port_pin >> 4;
    uint8_t pin = port_pin & 0x0FU;

    enable_port_clock(port_index);
    GPIO_TypeDef *port = port_from_index(port_index);

    port->MODER &= ~(3UL << (pin * 2));
    port->MODER |= (1UL << (pin * 2)); /* 01 = sortie push-pull */
}

void gpio_write(uint8_t port_pin, bool level) {
    uint8_t port_index = port_pin >> 4;
    uint8_t pin = port_pin & 0x0FU;
    GPIO_TypeDef *port = port_from_index(port_index);

    if (level) { port->BSRR = (1UL << pin); }
    else { port->BSRR = (1UL << (pin + 16)); }
}
