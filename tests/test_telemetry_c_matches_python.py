"""Vérifie que le protocole de télémétrie embarqué
(firmware/Core/Src/telemetry.c) produit et parse exactement les mêmes
trames que la version Python."""
from __future__ import annotations

import ctypes
import subprocess
from pathlib import Path

import pytest

from hab_sim.telemetry import TelemetryFrame, encode, decode

REPO_ROOT = Path(__file__).resolve().parent.parent
FIRMWARE_SRC = REPO_ROOT / "firmware" / "Core" / "Src" / "telemetry.c"
FIRMWARE_INC = REPO_ROOT / "firmware" / "Core" / "Inc"


class TmFrame(ctypes.Structure):
    _fields_ = [
        ("payload_id", ctypes.c_char * 16),
        ("sequence", ctypes.c_uint32),
        ("latitude_deg", ctypes.c_double),
        ("longitude_deg", ctypes.c_double),
        ("altitude_m", ctypes.c_double),
        ("temperature_c", ctypes.c_double),
        ("humidity_pct", ctypes.c_double),
        ("pressure_hpa", ctypes.c_double),
        ("battery_mv", ctypes.c_uint16),
    ]


@pytest.fixture(scope="module")
def tm_lib(tmp_path_factory):
    out_dir = tmp_path_factory.mktemp("telemetry_native")
    lib_path = out_dir / "libtelemetry.so"
    subprocess.run(
        ["gcc", "-shared", "-fPIC", "-O2",
         "-o", str(lib_path), str(FIRMWARE_SRC), f"-I{FIRMWARE_INC}"],
        check=True,
    )
    lib = ctypes.CDLL(str(lib_path))
    lib.tm_encode.argtypes = [ctypes.POINTER(TmFrame), ctypes.c_char_p, ctypes.c_int]
    lib.tm_encode.restype = ctypes.c_int
    lib.tm_decode.argtypes = [ctypes.c_char_p, ctypes.POINTER(TmFrame)]
    lib.tm_decode.restype = ctypes.c_bool
    return lib


SAMPLE = TelemetryFrame(
    payload_id="HAB1", sequence=42,
    latitude_deg=48.858222, longitude_deg=2.2945,
    altitude_m=18345.2, temperature_c=-52.3, humidity_pct=12.5,
    pressure_hpa=68.4, battery_mv=3050,
)


def test_c_encode_matches_python_encode(tm_lib):
    c_frame = TmFrame(
        payload_id=SAMPLE.payload_id.encode(), sequence=SAMPLE.sequence,
        latitude_deg=SAMPLE.latitude_deg, longitude_deg=SAMPLE.longitude_deg,
        altitude_m=SAMPLE.altitude_m, temperature_c=SAMPLE.temperature_c,
        humidity_pct=SAMPLE.humidity_pct, pressure_hpa=SAMPLE.pressure_hpa,
        battery_mv=SAMPLE.battery_mv,
    )
    buf = ctypes.create_string_buffer(128)
    n = tm_lib.tm_encode(ctypes.byref(c_frame), buf, 128)
    c_encoded = buf.value.decode()

    py_encoded = encode(SAMPLE)
    assert n > 0
    assert c_encoded.strip() == py_encoded.strip()


def test_c_decode_matches_python_decode(tm_lib):
    line = encode(SAMPLE)
    py_decoded = decode(line)

    c_frame = TmFrame()
    ok = tm_lib.tm_decode(line.strip().encode(), ctypes.byref(c_frame))

    assert ok
    assert py_decoded is not None
    assert c_frame.payload_id.decode() == py_decoded.payload_id
    assert c_frame.sequence == py_decoded.sequence
    assert c_frame.latitude_deg == pytest.approx(py_decoded.latitude_deg, abs=1e-6)
    assert c_frame.altitude_m == pytest.approx(py_decoded.altitude_m, abs=1e-6)
    assert c_frame.battery_mv == py_decoded.battery_mv


def test_c_decode_rejects_bad_checksum(tm_lib):
    line = encode(SAMPLE).strip()
    corrupted = line.rsplit("*", 1)[0] + "*00"
    c_frame = TmFrame()
    ok = tm_lib.tm_decode(corrupted.encode(), ctypes.byref(c_frame))
    assert not ok
