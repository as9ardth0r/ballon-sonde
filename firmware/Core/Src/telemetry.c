#include "telemetry.h"
#include <string.h>
#include <stdio.h>

static uint8_t checksum_of(const char *s, int len) {
    uint8_t v = 0;
    for (int i = 0; i < len; i++) { v ^= (uint8_t)s[i]; }
    return v;
}

int tm_encode(const tm_frame_t *f, char *out, int out_size) {
    char payload[100];
    int payload_len = snprintf(payload, sizeof(payload),
        "$$%s,%lu,%.6f,%.6f,%.1f,%.1f,%.1f,%.1f,%u",
        f->payload_id, (unsigned long)f->sequence,
        f->latitude_deg, f->longitude_deg, f->altitude_m,
        f->temperature_c, f->humidity_pct, f->pressure_hpa,
        (unsigned)f->battery_mv);

    if (payload_len <= 0 || payload_len >= (int)sizeof(payload)) return 0;

    uint8_t cs = checksum_of(payload, payload_len);
    int total = snprintf(out, (size_t)out_size, "%s*%02X\n", payload, cs);
    if (total <= 0 || total >= out_size) return 0;
    return total;
}

/* Parseur flottant manuel (double précision) : évite toute dépendance à
 * scanf("%lf", ...), dont le support sous newlib-nano n'est pas garanti
 * selon les options de link (voir Makefile, -u _printf_float ne couvre
 * que l'écriture, pas scanf). Double plutôt que float : une latitude/
 * longitude à 6 décimales dépasse la précision de float32 (~7 chiffres
 * significatifs au total) — un bug réel attrapé par
 * tests/test_telemetry_c_matches_python.py avant cette correction. */
static bool parse_double(const char **s, double *out) {
    const char *p = *s;
    bool negative = false;
    if (*p == '-') { negative = true; p++; }
    if ((*p < '0' || *p > '9') && *p != '.') return false;

    double value = 0.0;
    while (*p >= '0' && *p <= '9') { value = value * 10.0 + (double)(*p - '0'); p++; }

    if (*p == '.') {
        p++;
        double frac = 0.1;
        while (*p >= '0' && *p <= '9') {
            value += (double)(*p - '0') * frac;
            frac *= 0.1;
            p++;
        }
    }
    *out = negative ? -value : value;
    *s = p;
    return true;
}

static bool parse_uint(const char **s, unsigned long *out) {
    const char *p = *s;
    if (*p < '0' || *p > '9') return false;
    unsigned long value = 0;
    while (*p >= '0' && *p <= '9') { value = value * 10 + (unsigned long)(*p - '0'); p++; }
    *out = value;
    *s = p;
    return true;
}

bool tm_decode(const char *line, tm_frame_t *out) {
    if (line[0] != '$' || line[1] != '$') return false;

    const char *star = strchr(line, '*');
    if (star == NULL) return false;

    int payload_len = (int)(star - line);
    uint8_t computed = checksum_of(line, payload_len);

    unsigned int expected;
    if (sscanf(star + 1, "%2x", &expected) != 1) return false;
    if (computed != (uint8_t)expected) return false;

    const char *p = line + 2; /* après "$$" */

    /* payload_id : tout jusqu'à la prochaine virgule */
    int id_len = 0;
    while (p[id_len] != ',' && id_len < (int)sizeof(out->payload_id) - 1) { id_len++; }
    if (p[id_len] != ',') return false;
    memcpy(out->payload_id, p, (size_t)id_len);
    out->payload_id[id_len] = '\0';
    p += id_len + 1;

    unsigned long seq;
    if (!parse_uint(&p, &seq)) return false;
    if (*p++ != ',') return false;
    out->sequence = (uint32_t)seq;

    double lat, lon, alt, temp, hum, press;
    if (!parse_double(&p, &lat)) return false;
    if (*p++ != ',') return false;
    if (!parse_double(&p, &lon)) return false;
    if (*p++ != ',') return false;
    if (!parse_double(&p, &alt)) return false;
    if (*p++ != ',') return false;
    if (!parse_double(&p, &temp)) return false;
    if (*p++ != ',') return false;
    if (!parse_double(&p, &hum)) return false;
    if (*p++ != ',') return false;
    if (!parse_double(&p, &press)) return false;
    if (*p++ != ',') return false;

    unsigned long batt;
    if (!parse_uint(&p, &batt)) return false;
    if (p != star) return false;

    out->latitude_deg = lat;
    out->longitude_deg = lon;
    out->altitude_m = alt;
    out->temperature_c = temp;
    out->humidity_pct = hum;
    out->pressure_hpa = press;
    out->battery_mv = (uint16_t)batt;
    return true;
}
