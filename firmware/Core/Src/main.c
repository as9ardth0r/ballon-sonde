#include "stm32l0xx.h"
#include "clock.h"
#include "gpio.h"
#include "i2c_bus.h"
#include "spi1_bus.h"
#include "bme280.h"
#include "gps_uart.h"
#include "lora_sx1276.h"
#include "telemetry.h"
#include "nmea.h"
#include "ubx.h"
#include <string.h>

static void delay_loops(uint32_t n) {
    for (volatile uint32_t i = 0; i < n; i++) { }
}

int main(void) {
    clock_init_hsi16();
    i2c1_init();
    gps_uart_init(9600); /* baud NMEA par défaut de la plupart des modules u-blox */

    bool bme_ok = bme280_init();
    bool lora_ok = lora_init();
    (void)bme_ok;
    (void)lora_ok; /* TODO: remonter un état de santé capteurs (LED) plutôt
                     * que d'ignorer silencieusement un échec d'init */

    /* Configure le mode dynamique "Airborne <1g" du GPS — nécessaire
     * pour un vol ballon-sonde (voir docs/hardware.md et ubx.h). Ne
     * contourne pas la limite COCOM (déjà contournée nativement par les
     * modules u-blox 6/7/8/M10 via une logique ET), améliore juste la
     * qualité du fix en dynamique verticale rapide. */
    uint8_t ubx_frame[64];
    int ubx_len = ubx_build_cfg_nav5(ubx_frame, sizeof(ubx_frame), UBX_DYN_MODEL_AIRBORNE_1G);
    if (ubx_len > 0) {
        gps_send_ubx_frame(ubx_frame, (size_t)ubx_len);
    }

    tm_frame_t frame = {0};
    strncpy(frame.payload_id, "HAB1", sizeof(frame.payload_id) - 1);
    frame.payload_id[sizeof(frame.payload_id) - 1] = '\0';
    uint32_t sequence = 0;

    gga_fix_t last_fix = {0};
    bool have_fix = false;
    char gps_line[96];

    while (1) {
        /* draine toutes les trames NMEA disponibles, garde le dernier fix valide */
        while (gps_uart_poll_line(gps_line, sizeof(gps_line))) {
            gga_fix_t fix;
            if (nmea_parse_gga(gps_line, &fix)) {
                last_fix = fix;
                have_fix = true;
            }
        }

        bme280_sample_t sample = {0};
        bool sample_ok = bme280_read(&sample);

        frame.sequence = sequence++;
        frame.latitude_deg = have_fix ? last_fix.latitude_deg : 0.0;
        frame.longitude_deg = have_fix ? last_fix.longitude_deg : 0.0;
        /* altitude : GPS au-delà de ~9000 m (limite de plage du BME280,
         * voir docs/hardware.md), pression BME280 en-deçà si un fix GPS
         * n'est pas encore acquis */
        frame.altitude_m = have_fix ? last_fix.altitude_m : 0.0;
        frame.temperature_c = sample_ok ? sample.temperature_c : 0.0;
        frame.humidity_pct = sample_ok ? sample.humidity_pct : 0.0;
        frame.pressure_hpa = sample_ok ? sample.pressure_hpa : 0.0;
        frame.battery_mv = 0; /* TODO: ADC sur pont diviseur, non câblé dans ce squelette */

        char line[128];
        int len = tm_encode(&frame, line, sizeof(line));
        if (len > 0 && lora_ok) {
            lora_send((const uint8_t *)line, (size_t)len);
        }

        delay_loops(4000000U); /* cadence approximative, pas un timer précis —
                                  * voir README pour ce que ça implique */
    }
}
