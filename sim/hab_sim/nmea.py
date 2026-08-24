"""Parseur de trames NMEA GGA (position/altitude GPS). Format standard,
identique à celui émis par tous les modules GPS NMEA (u-blox compris,
voir docs/hardware.md). Miroir de firmware/Core/Src/nmea.c.

Exemple canonique (utilisé dans les tests) :
  $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
"""
from __future__ import annotations

from dataclasses import dataclass


@dataclass
class GgaFix:
    latitude_deg: float
    longitude_deg: float
    fix_quality: int
    num_satellites: int
    altitude_m: float


def _checksum(payload: str) -> int:
    value = 0
    for ch in payload:
        value ^= ord(ch)
    return value & 0xFF


def _nmea_coord_to_decimal(raw: str, minutes_digits: int) -> float:
    """Convertit ddmm.mmmm (ou dddmm.mmmm pour la longitude) en degrés
    décimaux. `minutes_digits` = nombre de chiffres avant le point qui
    appartiennent aux minutes (2 pour la latitude, 2 pour la longitude —
    seul le nombre de chiffres de degrés change)."""
    dot = raw.index(".")
    degrees = int(raw[: dot - minutes_digits])
    minutes = float(raw[dot - minutes_digits:])
    return degrees + minutes / 60.0


def parse_gga(sentence: str) -> GgaFix | None:
    """Parse une trame $GPGGA/$GNGGA complète (avec '$' et checksum).
    Retourne None si le checksum est invalide, le fix absent (quality=0),
    ou le format inattendu — ne lève jamais d'exception sur une entrée
    malformée (bruit radio possible sur le lien série GPS)."""
    sentence = sentence.strip()
    if not sentence.startswith("$") or "*" not in sentence:
        return None

    body, _, checksum_hex = sentence[1:].partition("*")
    try:
        expected = int(checksum_hex[:2], 16)
    except ValueError:
        return None
    if _checksum(body) != expected:
        return None

    fields = body.split(",")
    if len(fields) < 10 or not fields[0].endswith("GGA"):
        return None

    try:
        lat_raw, lat_hemi = fields[2], fields[3]
        lon_raw, lon_hemi = fields[4], fields[5]
        fix_quality = int(fields[6]) if fields[6] else 0
        num_satellites = int(fields[7]) if fields[7] else 0
        altitude_m = float(fields[9]) if fields[9] else 0.0

        if fix_quality == 0 or not lat_raw or not lon_raw:
            return None

        latitude = _nmea_coord_to_decimal(lat_raw, minutes_digits=2)
        if lat_hemi == "S":
            latitude = -latitude
        longitude = _nmea_coord_to_decimal(lon_raw, minutes_digits=2)
        if lon_hemi == "W":
            longitude = -longitude
    except (ValueError, IndexError):
        return None

    return GgaFix(latitude, longitude, fix_quality, num_satellites, altitude_m)
