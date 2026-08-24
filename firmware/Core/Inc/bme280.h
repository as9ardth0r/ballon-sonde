/**
 * bme280.h — pilote complet BME280 (température/pression/humidité).
 * Contrairement au VL53L1X du projet nanodrone, le BME280 a un registre
 * et une formule de compensation entièrement documentés publiquement
 * (datasheet Bosch BST-BME280-DS002) : ce pilote est donc complet, pas
 * un squelette avec point d'intégration externe.
 */
#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include <stdbool.h>

#define BME280_I2C_ADDR 0x76U /* 0x77 si SDO tiré au VDD plutôt qu'à la masse */

typedef struct {
    double temperature_c;
    double pressure_hpa;
    double humidity_pct;
} bme280_sample_t;

bool bme280_init(void);
bool bme280_read(bme280_sample_t *out);

#endif /* BME280_H */
