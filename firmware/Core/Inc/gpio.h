#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

#define GPIO_PIN(port_letter, pin_num) ((uint8_t)((((port_letter) - 'A') << 4) | (pin_num)))

void gpio_init_output(uint8_t port_pin);
void gpio_write(uint8_t port_pin, bool level);

#endif /* GPIO_H */
