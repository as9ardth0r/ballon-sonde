/**
 * lora_sx1276.h — pilote SX1276 (module RFM95W). Registres et séquence
 * d'initialisation documentés publiquement (Semtech SX1276 datasheet),
 * pilote complet comme bme280.h — pas un squelette.
 */
#ifndef LORA_SX1276_H
#define LORA_SX1276_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

bool lora_init(void);
bool lora_send(const uint8_t *data, size_t len);

#endif /* LORA_SX1276_H */
