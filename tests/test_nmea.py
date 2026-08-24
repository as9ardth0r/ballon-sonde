import pytest

from hab_sim.nmea import parse_gga

# Exemple canonique (Wikipedia/NMEA reference) — checksum 0x47 vérifié
# par calcul, pas recopié à l'aveugle.
CANONICAL_GGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"


def test_parses_canonical_example():
    fix = parse_gga(CANONICAL_GGA)
    assert fix is not None
    assert fix.latitude_deg == pytest.approx(48 + 7.038 / 60.0, abs=1e-6)
    assert fix.longitude_deg == pytest.approx(11 + 31.000 / 60.0, abs=1e-6)
    assert fix.fix_quality == 1
    assert fix.num_satellites == 8
    assert fix.altitude_m == pytest.approx(545.4)


def test_rejects_bad_checksum():
    corrupted = CANONICAL_GGA[:-2] + "00"
    assert parse_gga(corrupted) is None


def test_rejects_no_fix():
    no_fix = "$GPGGA,123519,4807.038,N,01131.000,E,0,00,,,,M,,M,,*"
    # checksum recalculé pour ce cas précis
    body = no_fix[1:no_fix.index("*")]
    cs = 0
    for c in body:
        cs ^= ord(c)
    no_fix = no_fix[:no_fix.index("*") + 1] + f"{cs:02X}"
    assert parse_gga(no_fix) is None


def test_rejects_garbage():
    assert parse_gga("not a sentence") is None
    assert parse_gga("") is None
    assert parse_gga("$GPRMC,123519*XX") is None  # mauvais type de trame


def test_southern_western_hemisphere_gives_negative_coordinates():
    body = "GPGGA,123519,4807.038,S,01131.000,W,1,08,0.9,545.4,M,46.9,M,,"
    cs = 0
    for c in body:
        cs ^= ord(c)
    sentence = f"${body}*{cs:02X}"
    fix = parse_gga(sentence)
    assert fix is not None
    assert fix.latitude_deg < 0
    assert fix.longitude_deg < 0
