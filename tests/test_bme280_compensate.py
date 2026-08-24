import pytest

from hab_sim.bme280_compensate import (
    CalibrationData, compensate_temperature, compensate_pressure,
    compensate_humidity, pressure_to_altitude_m,
)

# Coefficients plausibles (ordre de grandeur réel d'un BME280 calibré en
# usine), utilisés pour des tests de cohérence — pas une reproduction
# certifiée de l'exemple chiffré de la datasheet Bosch (voir README).
SAMPLE_CAL = CalibrationData(
    dig_T1=27504, dig_T2=26435, dig_T3=-1000,
    dig_P1=36477, dig_P2=-10685, dig_P3=3024, dig_P4=2855, dig_P5=140,
    dig_P6=-7, dig_P7=15500, dig_P8=-14600, dig_P9=6000,
    dig_H1=75, dig_H2=356, dig_H3=0, dig_H4=345, dig_H5=0, dig_H6=30,
)


def test_temperature_in_plausible_range():
    temp_c, t_fine = compensate_temperature(519888, SAMPLE_CAL)
    assert -50.0 < temp_c < 60.0
    assert isinstance(t_fine, float)


def test_temperature_is_monotonic_in_adc_value():
    t_low, _ = compensate_temperature(400000, SAMPLE_CAL)
    t_high, _ = compensate_temperature(600000, SAMPLE_CAL)
    assert t_high > t_low


def test_pressure_in_plausible_range():
    _, t_fine = compensate_temperature(519888, SAMPLE_CAL)
    pressure_pa = compensate_pressure(415148, t_fine, SAMPLE_CAL)
    # gamme opérationnelle BME280 : 300-1100 hPa (30000-110000 Pa)
    assert 30000.0 < pressure_pa < 110000.0


def test_humidity_bounded_0_100():
    _, t_fine = compensate_temperature(519888, SAMPLE_CAL)
    humidity = compensate_humidity(32768, t_fine, SAMPLE_CAL)
    assert 0.0 <= humidity <= 100.0


def test_altitude_at_sea_level_pressure_is_zero():
    assert pressure_to_altitude_m(101325.0, sea_level_pa=101325.0) == pytest.approx(0.0, abs=1e-9)


def test_altitude_decreases_as_pressure_increases():
    alt_low_pressure = pressure_to_altitude_m(50000.0)
    alt_high_pressure = pressure_to_altitude_m(90000.0)
    assert alt_low_pressure > alt_high_pressure  # moins de pression = plus haut


def test_altitude_5000m_matches_standard_atmosphere_approximately():
    # Atmosphère standard internationale (ISA) : ~54048 Pa à 5000 m.
    # Tolérance large (±150 m) car la formule barométrique simple ne
    # modélise pas les variations réelles de température avec l'altitude.
    altitude = pressure_to_altitude_m(54048.0)
    assert altitude == pytest.approx(5000.0, abs=150.0)
