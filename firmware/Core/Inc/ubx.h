/**
 * ubx.h — construction de la trame UBX-CFG-NAV5 (mode dynamique GPS
 * "Airborne"). Miroir de sim/hab_sim/ubx.py.
 */
#ifndef UBX_H
#define UBX_H

#include <stdint.h>
#include <stddef.h>

#define UBX_DYN_MODEL_AIRBORNE_1G 6

/* Écrit la trame dans `out` (taille `out_size`, doit faire au moins 44
 * octets). Retourne la longueur écrite, ou -1 si out_size est trop petit. */
int ubx_build_cfg_nav5(uint8_t *out, int out_size, uint8_t dyn_model);

#endif /* UBX_H */
