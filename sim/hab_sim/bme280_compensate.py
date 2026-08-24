"""Formules de compensation BME280 (température/pression/humidité),
transcrites de la section 4.2.3 de la datasheet Bosch (formules en
virgule flottante). Miroir exact de firmware/Core/Src/bme280_compensate.c
— voir tests/test_bme280_c_matches_python.py.

Ce module ne fait QUE la compensation mathématique (ADC brut + coefficients
de calibration -> valeurs physiques) ; la lecture I2C réelle des registres
est dans bme280_i2c.py / bme280.c côté firmware.
"""
from __future__ import annotations

from dataclasses import dataclass


@dataclass
class CalibrationData:
    dig_T1: int; dig_T2: int; dig_T3: int
    dig_P1: int; dig_P2: int; dig_P3: int; dig_P4: int; dig_P5: int
    dig_P6: int; dig_P7: int; dig_P8: int; dig_P9: int
    dig_H1: int; dig_H2: int; dig_H3: int; dig_H4: int; dig_H5: int; dig_H6: int


def compensate_temperature(adc_T: int, cal: CalibrationData) -> tuple[float, float]:
    """Retourne (température en °C, t_fine) — t_fine est réutilisé par
    les compensations pression/humidité, comme l'exige la datasheet."""
    var1 = (adc_T / 16384.0 - cal.dig_T1 / 1024.0) * cal.dig_T2
    var2 = ((adc_T / 131072.0 - cal.dig_T1 / 8192.0)
            * (adc_T / 131072.0 - cal.dig_T1 / 8192.0)) * cal.dig_T3
    t_fine = var1 + var2
    return t_fine / 5120.0, t_fine


def compensate_pressure(adc_P: int, t_fine: float, cal: CalibrationData) -> float:
    """Retourne la pression en Pa. 0.0 si le calcul est indéterminé
    (var1 == 0, cas dégénéré évité par la datasheet elle-même)."""
    var1 = t_fine / 2.0 - 64000.0
    var2 = var1 * var1 * cal.dig_P6 / 32768.0
    var2 = var2 + var1 * cal.dig_P5 * 2.0
    var2 = var2 / 4.0 + cal.dig_P4 * 65536.0
    var1 = (cal.dig_P3 * var1 * var1 / 524288.0 + cal.dig_P2 * var1) / 524288.0
    var1 = (1.0 + var1 / 32768.0) * cal.dig_P1

    if var1 == 0.0:
        return 0.0

    p = 1048576.0 - adc_P
    p = (p - var2 / 4096.0) * 6250.0 / var1
    var1 = cal.dig_P9 * p * p / 2147483648.0
    var2 = p * cal.dig_P8 / 32768.0
    p = p + (var1 + var2 + cal.dig_P7) / 16.0
    return p


def compensate_humidity(adc_H: int, t_fine: float, cal: CalibrationData) -> float:
    """Retourne l'humidité relative en %, bornée à [0, 100]."""
    var_h = t_fine - 76800.0
    var_h = ((adc_H - (cal.dig_H4 * 64.0 + cal.dig_H5 / 16384.0 * var_h))
             * (cal.dig_H2 / 65536.0 * (1.0 + cal.dig_H6 / 67108864.0 * var_h
                * (1.0 + cal.dig_H3 / 67108864.0 * var_h))))
    var_h = var_h * (1.0 - cal.dig_H1 * var_h / 524288.0)
    return max(0.0, min(100.0, var_h))


def pressure_to_altitude_m(pressure_pa: float, sea_level_pa: float = 101325.0) -> float:
    """Formule barométrique standard (approximation atmosphère standard
    internationale) — valable jusqu'à ~11 km (limite de la troposphère) ;
    au-delà, l'altitude GPS est la seule source fiable (voir README :
    la gamme de pression du BME280 elle-même, 300-1100 hPa, correspond
    à ~9000 m de toute façon)."""
    return 44330.0 * (1.0 - (pressure_pa / sea_level_pa) ** (1.0 / 5.255))
