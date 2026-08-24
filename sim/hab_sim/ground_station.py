"""Station sol : décode la télémétrie reçue par un module LoRa relié en
série (voir docs/ground-station.md pour le matériel), journalise dans un
CSV, affiche l'état courant. Réutilise directement hab_sim.telemetry —
même code de décodage que celui déjà validé contre le firmware C.
"""
from __future__ import annotations

import argparse
import csv
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

from hab_sim.telemetry import TelemetryFrame, decode

CSV_HEADER = [
    "received_at_utc", "payload_id", "sequence",
    "latitude_deg", "longitude_deg", "altitude_m",
    "temperature_c", "humidity_pct", "pressure_hpa", "battery_mv",
]


def frame_to_row(frame: TelemetryFrame, received_at: datetime) -> list:
    return [
        received_at.isoformat(), frame.payload_id, frame.sequence,
        frame.latitude_deg, frame.longitude_deg, frame.altitude_m,
        frame.temperature_c, frame.humidity_pct, frame.pressure_hpa, frame.battery_mv,
    ]


def process_line(line: str, csv_writer, received_at: datetime | None = None) -> TelemetryFrame | None:
    """Traite une ligne reçue : décode, journalise si valide. Retourne la
    trame décodée (ou None si la ligne était invalide/du bruit radio —
    normal et attendu sur un lien LoRa, pas une erreur à remonter)."""
    frame = decode(line)
    if frame is None:
        return None
    received_at = received_at or datetime.now(timezone.utc)
    csv_writer.writerow(frame_to_row(frame, received_at))
    return frame


def format_status_line(frame: TelemetryFrame) -> str:
    return (
        f"#{frame.sequence:>5} | {frame.latitude_deg:.6f},{frame.longitude_deg:.6f} "
        f"| alt {frame.altitude_m:>7.1f} m | {frame.temperature_c:>6.1f}°C "
        f"| {frame.humidity_pct:>5.1f}% | {frame.pressure_hpa:>7.1f} hPa "
        f"| batt {frame.battery_mv} mV"
    )


def run(serial_port: str, baudrate: int, log_path: Path) -> None:
    import serial  # import différé : dépendance optionnelle, seulement pour l'usage réel

    log_path.parent.mkdir(parents=True, exist_ok=True)
    write_header = not log_path.exists()

    with open(log_path, "a", newline="") as log_file:
        writer = csv.writer(log_file)
        if write_header:
            writer.writerow(CSV_HEADER)
            log_file.flush()

        print(f"[ground-station] écoute sur {serial_port} @ {baudrate} bauds, "
              f"journal -> {log_path}")

        with serial.Serial(serial_port, baudrate, timeout=1) as ser:
            while True:
                raw = ser.readline()
                if not raw:
                    continue
                line = raw.decode(errors="ignore").strip()
                if not line:
                    continue

                frame = process_line(line, writer)
                log_file.flush()
                if frame is not None:
                    print(format_status_line(frame))
                else:
                    print(f"[bruit/invalide] {line!r}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="/dev/ttyUSB0",
                         help="port série du dongle LoRa (ex. COM3 sous Windows)")
    parser.add_argument("--baud", type=int, default=9600)
    parser.add_argument("--log", type=Path, default=Path("ground_station_log.csv"))
    args = parser.parse_args()

    try:
        run(args.port, args.baud, args.log)
    except KeyboardInterrupt:
        print("\n[ground-station] arrêt demandé.")
        sys.exit(0)


if __name__ == "__main__":
    main()
