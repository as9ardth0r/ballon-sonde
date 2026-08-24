import csv
from datetime import datetime, timezone

from hab_sim.ground_station import CSV_HEADER, process_line
from hab_sim.telemetry import TelemetryFrame, encode

SAMPLE = TelemetryFrame(
    payload_id="HAB1", sequence=7,
    latitude_deg=48.858222, longitude_deg=2.2945,
    altitude_m=12500.0, temperature_c=-40.2, humidity_pct=8.0,
    pressure_hpa=180.5, battery_mv=3100,
)


class FakeWriter:
    """Remplace un csv.writer réel pour observer ce qui serait écrit,
    sans toucher au disque."""
    def __init__(self):
        self.rows = []

    def writerow(self, row):
        self.rows.append(row)


def test_process_line_logs_valid_frame():
    writer = FakeWriter()
    line = encode(SAMPLE)
    frame = process_line(line, writer, received_at=datetime(2026, 8, 24, tzinfo=timezone.utc))

    assert frame is not None
    assert len(writer.rows) == 1
    row = writer.rows[0]
    assert row[1] == "HAB1"  # payload_id
    assert row[2] == 7        # sequence


def test_process_line_ignores_noise_without_logging():
    writer = FakeWriter()
    result = process_line("bruit radio aléatoire", writer)
    assert result is None
    assert len(writer.rows) == 0


def test_process_line_ignores_corrupted_checksum():
    writer = FakeWriter()
    corrupted = encode(SAMPLE).rsplit("*", 1)[0] + "*00\n"
    result = process_line(corrupted, writer)
    assert result is None
    assert len(writer.rows) == 0


def test_csv_header_matches_row_length():
    writer = FakeWriter()
    process_line(encode(SAMPLE), writer)
    assert len(writer.rows[0]) == len(CSV_HEADER)


def test_full_csv_roundtrip(tmp_path):
    """Écrit un vrai fichier CSV et le relit — vérifie que le format
    produit est un CSV valide et exploitable, pas juste des appels
    writerow() qui s'assemblent bien en mémoire."""
    csv_path = tmp_path / "log.csv"
    with open(csv_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(CSV_HEADER)
        process_line(encode(SAMPLE), writer,
                      received_at=datetime(2026, 8, 24, 10, 0, tzinfo=timezone.utc))

    with open(csv_path) as f:
        rows = list(csv.reader(f))

    assert rows[0] == CSV_HEADER
    assert rows[1][1] == "HAB1"
    assert float(rows[1][3]) == SAMPLE.latitude_deg
