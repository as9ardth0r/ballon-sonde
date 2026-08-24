/**
 * gps_uart.h — pilote USART2 pour le module GPS (NMEA en réception,
 * envoi de la trame UBX-CFG-NAV5 au démarrage pour le mode vol —
 * voir docs/hardware.md). PA2=TX, PA3=RX (AF4).
 */
#ifndef GPS_UART_H
#define GPS_UART_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void gps_uart_init(uint32_t baudrate);

/* Envoie la trame de configuration UBX (mode dynamique "airborne <1g")
 * construite par ubx.c. */
void gps_send_ubx_frame(const uint8_t *frame, size_t len);

/* Poll non-bloquant, même sémantique que uart3_poll_line côté nanodrone :
 * accumule les caractères NMEA jusqu'à '\n', copie la ligne complète
 * dans line_out et retourne true. */
bool gps_uart_poll_line(char *line_out, size_t max_len);

#endif /* GPS_UART_H */
