"""Format du paquet de télémétrie envoyé par LoRa. Trame texte ASCII
compacte (facile à décoder à la main sur un récepteur LoRa générique
sans logiciel dédié), inspirée des formats utilisés par la communauté
ballon-sonde amateur (UKHAS-like), simplifiée pour ce projet.

Format : "$$<id>,<seq>,<lat>,<lon>,<alt_m>,<temp_c>,<hum_pct>,<pressure_hpa>,<batt_mv>*<checksum_hex>\n"
Miroir de firmware/Core/Src/telemetry.c.
"""
from __future__ import annotations

from dataclasses import dataclass


@dataclass
class TelemetryFrame:
    payload_id: str
    sequence: int
    latitude_deg: float
    longitude_deg: float
    altitude_m: float
    temperature_c: float
    humidity_pct: float
    pressure_hpa: float
    battery_mv: int


def _checksum(payload: str) -> int:
    value = 0
    for ch in payload:
        value ^= ord(ch)
    return value & 0xFF


def encode(frame: TelemetryFrame) -> str:
    payload = (
        f"$${frame.payload_id},{frame.sequence},"
        f"{frame.latitude_deg:.6f},{frame.longitude_deg:.6f},{frame.altitude_m:.1f},"
        f"{frame.temperature_c:.1f},{frame.humidity_pct:.1f},{frame.pressure_hpa:.1f},"
        f"{frame.battery_mv}"
    )
    return f"{payload}*{_checksum(payload):02X}\n"


def decode(line: str) -> TelemetryFrame | None:
    """Retourne None sur trame invalide (checksum, format) plutôt que de
    lever une exception — un récepteur LoRa au sol reçoit du bruit sur
    un lien radio, ça doit être toléré, pas fatal."""
    line = line.strip()
    if not line.startswith("$$") or "*" not in line:
        return None

    payload, _, checksum_hex = line.partition("*")
    try:
        expected = int(checksum_hex, 16)
    except ValueError:
        return None
    if _checksum(payload) != expected:
        return None

    fields = payload[2:].split(",")
    if len(fields) != 9:
        return None

    try:
        return TelemetryFrame(
            payload_id=fields[0],
            sequence=int(fields[1]),
            latitude_deg=float(fields[2]),
            longitude_deg=float(fields[3]),
            altitude_m=float(fields[4]),
            temperature_c=float(fields[5]),
            humidity_pct=float(fields[6]),
            pressure_hpa=float(fields[7]),
            battery_mv=int(fields[8]),
        )
    except ValueError:
        return None
