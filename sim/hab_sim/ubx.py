"""Construction de trames UBX (protocole binaire u-blox), en particulier
UBX-CFG-NAV5 pour configurer le modèle dynamique "Airborne <1g" —
nécessaire pour un vol ballon-sonde : sans ce réglage, le firmware GPS
peut appliquer un modèle de dynamique "portable" mal adapté à une ascension
verticale rapide suivie d'une chute libre au moment de l'éclatement.

Ce n'est PAS ce qui contourne la limite COCOM (18 km / 515 m/s) — les
modules u-blox 6/7/8/M10 la contournent nativement par une logique ET
(les deux doivent être dépassés simultanément, jamais le cas pour un
ballon). Le mode Airborne améliore la qualité du fix en dynamique
verticale rapide, ce sont deux sujets différents — voir docs/hardware.md.

Miroir de firmware/Core/Src/ubx.c.
"""
from __future__ import annotations

import struct

UBX_SYNC_1 = 0xB5
UBX_SYNC_2 = 0x62
UBX_CLASS_CFG = 0x06
UBX_ID_CFG_NAV5 = 0x24

DYN_MODEL_AIRBORNE_1G = 6  # recommandé pour ballon-sonde (accélération < 1g)


def _ubx_checksum(class_id_len_payload: bytes) -> tuple[int, int]:
    """Somme de contrôle Fletcher 8 bits, telle que spécifiée par le
    protocole UBX (interface description u-blox, section "UBX Checksum")."""
    ck_a, ck_b = 0, 0
    for byte in class_id_len_payload:
        ck_a = (ck_a + byte) & 0xFF
        ck_b = (ck_b + ck_a) & 0xFF
    return ck_a, ck_b


def build_ubx_cfg_nav5(dyn_model: int = DYN_MODEL_AIRBORNE_1G) -> bytes:
    """Construit une trame UBX-CFG-NAV5 minimale qui ne modifie QUE le
    modèle dynamique (mask=0x0001), le reste des champs à zéro/valeur
    neutre plutôt que d'imposer des réglages non voulus."""
    mask = 0x0001  # bit 0 : appliquer dynModel uniquement
    payload = struct.pack(
        "<HBBiIbBHHHHBBBBHHBxxxxx",
        mask, dyn_model, 0,   # mask, dynModel, fixMode (0 = ne pas changer... voir note)
        0, 0,                  # fixedAlt, fixedAltVar
        0, 0,                   # minElev, drLimit
        0, 0, 0, 0,               # pDop, tDop, pAcc, tAcc
        0, 0, 0, 0,                # staticHoldThresh, dgnssTimeout, cnoThreshNumSVs, cnoThresh
        0,                          # reserved2 (H)
        0,                           # staticHoldMaxDist (H)
        0,                            # utcStandard
    )
    assert len(payload) == 36, f"payload UBX-CFG-NAV5 doit faire 36 octets, ici {len(payload)}"

    length_bytes = struct.pack("<H", len(payload))
    body = bytes([UBX_CLASS_CFG, UBX_ID_CFG_NAV5]) + length_bytes + payload
    ck_a, ck_b = _ubx_checksum(body)

    return bytes([UBX_SYNC_1, UBX_SYNC_2]) + body + bytes([ck_a, ck_b])
