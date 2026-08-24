/**
 * nmea.h — parseur de trames NMEA GGA. Miroir de sim/hab_sim/nmea.py.
 */
#ifndef NMEA_H
#define NMEA_H

#include <stdbool.h>

typedef struct {
    double latitude_deg;
    double longitude_deg;
    int fix_quality;
    int num_satellites;
    double altitude_m;
} gga_fix_t;

/* Retourne false si checksum invalide, pas de fix (quality=0), ou
 * format inattendu. */
bool nmea_parse_gga(const char *sentence, gga_fix_t *out);

#endif /* NMEA_H */
