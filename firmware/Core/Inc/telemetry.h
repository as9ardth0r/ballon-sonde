/**
 * telemetry.h — encodage/décodage du paquet de télémétrie LoRa.
 * Indépendant du matériel, miroir de sim/hab_sim/telemetry.py — voir
 * tests/test_telemetry_c_matches_python.py.
 */
#ifndef HAB_TELEMETRY_H
#define HAB_TELEMETRY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char payload_id[16];
    uint32_t sequence;
    double latitude_deg;
    double longitude_deg;
    double altitude_m;
    double temperature_c;
    double humidity_pct;
    double pressure_hpa;
    uint16_t battery_mv;
} tm_frame_t;

/* Encode `frame` dans `out` (doit faire au moins 128 octets). Retourne
 * la longueur écrite (sans le '\0' final), ou 0 en cas d'erreur. */
int tm_encode(const tm_frame_t *frame, char *out, int out_size);

/* Décode une ligne reçue. Retourne false si le format ou le checksum
 * est invalide. */
bool tm_decode(const char *line, tm_frame_t *out);

#endif /* HAB_TELEMETRY_H */
