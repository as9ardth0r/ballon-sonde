/**
 * i2c_bus.h — pilote I2C1 pour le périphérique I2Cv2 du STM32L0 (bien
 * différent du I2Cv1 du STM32F405 utilisé sur le projet nanodrone —
 * transactions pilotées par CR2/NBYTES/AUTOEND, pas CCR/TRISE).
 */
#ifndef I2C_BUS_H
#define I2C_BUS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void i2c1_init(void);
bool i2c1_write(uint8_t addr7, const uint8_t *data, size_t len);
bool i2c1_write_read(uint8_t addr7, const uint8_t *reg, size_t reg_len,
                      uint8_t *data, size_t data_len);

#endif /* I2C_BUS_H */
