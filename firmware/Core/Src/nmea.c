#include "nmea.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t checksum_of(const char *s, int len) {
    uint8_t v = 0;
    for (int i = 0; i < len; i++) { v ^= (uint8_t)s[i]; }
    return v;
}

/* Copie le champ n (indexé à partir de 0, séparé par des virgules) de
 * `body` dans `out` (borné à out_size). Retourne la longueur du champ,
 * ou -1 si le champ n'existe pas. */
static int get_field(const char *body, int n, char *out, int out_size) {
    int field_idx = 0;
    const char *start = body;
    const char *p = body;

    while (field_idx < n) {
        if (*p == '\0') return -1;
        if (*p == ',') { field_idx++; start = p + 1; }
        p++;
    }
    p = start;
    int len = 0;
    while (*p != ',' && *p != '\0' && len < out_size - 1) { out[len++] = *p++; }
    out[len] = '\0';
    return len;
}

static double nmea_coord_to_decimal(const char *raw) {
    const char *dot = strchr(raw, '.');
    if (dot == NULL) return 0.0;
    int minutes_digits = 2;
    int int_len = (int)(dot - raw) - minutes_digits;
    if (int_len < 0) return 0.0;

    char deg_buf[8] = {0};
    memcpy(deg_buf, raw, (size_t)int_len);
    double degrees = atof(deg_buf);
    double minutes = atof(raw + int_len);
    return degrees + minutes / 60.0;
}

bool nmea_parse_gga(const char *sentence, gga_fix_t *out) {
    if (sentence[0] != '$') return false;
    const char *star = strchr(sentence, '*');
    if (star == NULL) return false;

    const char *body = sentence + 1;
    int body_len = (int)(star - body);
    uint8_t computed = checksum_of(body, body_len);

    unsigned int expected;
    if (sscanf(star + 1, "%2x", &expected) != 1) return false;
    if (computed != (uint8_t)expected) return false;

    char field[16];
    if (get_field(body, 0, field, sizeof(field)) < 0) return false;
    if (strstr(field, "GGA") == NULL) return false;

    char lat_raw[16], lat_hemi[2], lon_raw[16], lon_hemi[2];
    char quality_s[4], numsat_s[4], alt_s[16];

    if (get_field(body, 2, lat_raw, sizeof(lat_raw)) <= 0) return false;
    if (get_field(body, 3, lat_hemi, sizeof(lat_hemi)) <= 0) return false;
    if (get_field(body, 4, lon_raw, sizeof(lon_raw)) <= 0) return false;
    if (get_field(body, 5, lon_hemi, sizeof(lon_hemi)) <= 0) return false;
    if (get_field(body, 6, quality_s, sizeof(quality_s)) < 0) return false;
    if (get_field(body, 7, numsat_s, sizeof(numsat_s)) < 0) return false;
    if (get_field(body, 9, alt_s, sizeof(alt_s)) < 0) return false;

    int quality = atoi(quality_s);
    if (quality == 0) return false;

    out->fix_quality = quality;
    out->num_satellites = atoi(numsat_s);
    out->altitude_m = atof(alt_s);

    double lat = nmea_coord_to_decimal(lat_raw);
    if (lat_hemi[0] == 'S') lat = -lat;
    double lon = nmea_coord_to_decimal(lon_raw);
    if (lon_hemi[0] == 'W') lon = -lon;

    out->latitude_deg = lat;
    out->longitude_deg = lon;
    return true;
}
