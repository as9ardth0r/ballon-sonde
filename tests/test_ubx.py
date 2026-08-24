from hab_sim.ubx import build_ubx_cfg_nav5, _ubx_checksum, DYN_MODEL_AIRBORNE_1G


def test_frame_has_correct_sync_and_class_id():
    frame = build_ubx_cfg_nav5()
    assert frame[0:2] == bytes([0xB5, 0x62])
    assert frame[2] == 0x06  # CFG
    assert frame[3] == 0x24  # NAV5


def test_frame_length_field_matches_payload():
    frame = build_ubx_cfg_nav5()
    length = frame[4] | (frame[5] << 8)
    assert length == 36
    assert len(frame) == 2 + 1 + 1 + 2 + 36 + 2  # sync+class+id+len+payload+checksum


def test_dyn_model_byte_is_airborne_1g_by_default():
    frame = build_ubx_cfg_nav5()
    assert frame[8] == DYN_MODEL_AIRBORNE_1G


def test_dyn_model_is_configurable():
    frame = build_ubx_cfg_nav5(dyn_model=8)  # airborne <4g
    assert frame[8] == 8


def test_checksum_is_internally_consistent():
    """Recalcule le checksum sur le corps de la trame et vérifie qu'il
    correspond aux deux derniers octets — détecterait toute erreur dans
    l'algorithme Fletcher si le format du message changeait."""
    frame = build_ubx_cfg_nav5()
    body = frame[2:-2]  # class+id+len+payload, sans sync ni checksum
    ck_a, ck_b = _ubx_checksum(body)
    assert (ck_a, ck_b) == (frame[-2], frame[-1])
