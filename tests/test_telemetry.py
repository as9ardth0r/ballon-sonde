from hab_sim.telemetry import TelemetryFrame, encode, decode

SAMPLE = TelemetryFrame(
    payload_id="HAB1", sequence=42,
    latitude_deg=48.858222, longitude_deg=2.2945,
    altitude_m=18345.2, temperature_c=-52.3, humidity_pct=12.5,
    pressure_hpa=68.4, battery_mv=3050,
)


def test_encode_decode_roundtrip():
    frame = encode(SAMPLE)
    decoded = decode(frame)
    assert decoded is not None
    assert decoded.payload_id == SAMPLE.payload_id
    assert decoded.sequence == SAMPLE.sequence
    assert decoded.latitude_deg == SAMPLE.latitude_deg
    assert decoded.longitude_deg == SAMPLE.longitude_deg
    assert decoded.altitude_m == SAMPLE.altitude_m
    assert decoded.temperature_c == SAMPLE.temperature_c
    assert decoded.battery_mv == SAMPLE.battery_mv


def test_decode_rejects_bad_checksum():
    frame = encode(SAMPLE)
    corrupted = frame.rsplit("*", 1)[0] + "*00\n"
    assert decode(corrupted) is None


def test_decode_rejects_garbage():
    assert decode("noise") is None
    assert decode("") is None
    assert decode("$$HAB1,1,2*AA") is None  # champs manquants


def test_negative_temperature_and_coordinates_roundtrip():
    frame = TelemetryFrame(
        payload_id="HAB1", sequence=1,
        latitude_deg=-33.865, longitude_deg=-70.9,
        altitude_m=0.0, temperature_c=-60.0, humidity_pct=0.0,
        pressure_hpa=1013.0, battery_mv=3300,
    )
    decoded = decode(encode(frame))
    assert decoded is not None
    assert decoded.latitude_deg == frame.latitude_deg
    assert decoded.temperature_c == frame.temperature_c
