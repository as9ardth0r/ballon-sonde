#include "ubx.h"
#include <string.h>

#define UBX_SYNC_1 0xB5U
#define UBX_SYNC_2 0x62U
#define UBX_CLASS_CFG 0x06U
#define UBX_ID_CFG_NAV5 0x24U
#define PAYLOAD_LEN 36

int ubx_build_cfg_nav5(uint8_t *out, int out_size, uint8_t dyn_model) {
    const int total_len = 2 + 1 + 1 + 2 + PAYLOAD_LEN + 2;
    if (out_size < total_len) return -1;

    uint8_t payload[PAYLOAD_LEN];
    memset(payload, 0, sizeof(payload));
    payload[0] = 0x01U; /* mask low byte : bit0 = appliquer dynModel */
    payload[1] = 0x00U; /* mask high byte */
    payload[2] = dyn_model;
    /* le reste des 33 octets reste à zéro (mask=0x0001 -> seul dynModel
     * est réellement appliqué par le récepteur, le reste est ignoré) */

    int i = 0;
    out[i++] = UBX_SYNC_1;
    out[i++] = UBX_SYNC_2;
    out[i++] = UBX_CLASS_CFG;
    out[i++] = UBX_ID_CFG_NAV5;
    out[i++] = (uint8_t)(PAYLOAD_LEN & 0xFF);
    out[i++] = (uint8_t)((PAYLOAD_LEN >> 8) & 0xFF);

    int body_start = 2; /* class+id+len+payload commence à l'offset 2 */
    memcpy(&out[i], payload, PAYLOAD_LEN);
    i += PAYLOAD_LEN;

    uint8_t ck_a = 0, ck_b = 0;
    for (int k = body_start; k < i; k++) {
        ck_a = (uint8_t)(ck_a + out[k]);
        ck_b = (uint8_t)(ck_b + ck_a);
    }
    out[i++] = ck_a;
    out[i++] = ck_b;

    return i;
}
