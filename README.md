# ballon-sonde

### Charge utile pour ballon-sonde stratosphérique — STM32L0, BME280, GPS, LoRa

Mesure température/humidité/pression et position GPS pendant l'ascension,
transmet en continu par LoRa (pas besoin de récupérer la charge pour avoir
les données), pensé pour un vol de plusieurs heures sur piles en froid
stratosphérique.

## Ce qui est réel et vérifié (29 tests)

| Brique | Vérifié comment |
|---|---|
| **Compensation BME280** (température/pression/humidité) | Formules Bosch (datasheet §4.2.3). Test contre l'atmosphère standard internationale (5000 m ↔ 54048 Pa, ±150 m de tolérance) |
| **Parseur NMEA GGA** | Validé sur l'exemple canonique du standard NMEA, checksum recalculé par le code plutôt que recopié |
| **Trame UBX-CFG-NAV5** (mode vol GPS) | Structure et checksum Fletcher vérifiés |
| **Protocole de télémétrie LoRa** | Encodage/décodage C ↔ Python comparés via `ctypes` — **un vrai bug attrapé en cours de route** : `float` (simple précision) tronquait une latitude GPS à la 6ᵉ décimale, corrigé en `double` partout |
| **Station sol** (`ground_station.py`) | Décodage/journalisation CSV testés (trames valides, bruit radio, checksum corrompu, round-trip CSV complet) |
| **Firmware STM32L052** | Compile et **linke réellement** avec `arm-none-eabi-gcc` (46,5 Ko / 64 Ko de flash) contre les en-têtes CMSIS officiels ARM/ST |
| **Pilotes I2C, UART, SPI** | Écrits spécifiquement pour le périphérique I2Cv2/USART "nouvelle génération" du STM32L0 — différents de ceux du projet nanodrone (STM32F405, I2Cv1), pas une copie adaptée à la hâte |

## Ce qui n'est PAS vérifié

- **BME280 et LoRa (firmware)** : registres documentés publiquement, pilotes écrits en entier (contrairement au VL53L1X du nanodrone), mais pas testés sur un vrai capteur/module — aucun matériel disponible dans cet environnement de développement.
- **La formule de compensation BME280 côté firmware** (`bme280.c`) reprend la même structure que `bme280_compensate.py`, mais n'a pas été comparée numériquement via `ctypes` comme `telemetry.c` — à faire si une garantie plus forte est utile.
- **Aucun vol réel, aucune mesure sur banc.**

## Nomenclature, PCB, et calcul d'hydrogène

- **[docs/hardware.md](docs/hardware.md)** — nomenclature précise (référence,
  poids, rôle), plan de brochage, points de vigilance spécifiques à un vol
  stratosphérique (limite GPS, plage du BME280, piles), et **le calcul de
  quantité d'hydrogène** (~0,6-0,7 m³ pour cette charge utile — voir le
  détail du calcul et les réserves de sécurité/précision dans le document).
- **[docs/pcb.md](docs/pcb.md)** — traitement plus léger que le projet
  nanodrone (pas de courants forts) ; le vrai sujet ici est l'isolation
  thermique, pas le routage.
- **[docs/ground-station.md](docs/ground-station.md)** — matériel récepteur,
  suivi pendant le vol, logistique de récupération.

## Structure du dépôt

```
sim/hab_sim/
├── bme280_compensate.py   # compensation température/pression/humidité
├── nmea.py                 # parseur GGA
├── ubx.py                   # trame de config GPS (mode vol)
├── telemetry.py               # protocole LoRa
└── ground_station.py            # décodage + journalisation côté sol
firmware/
├── Core/Inc, Core/Src        # pilotes réels (I2Cv2, UART, SPI, BME280, SX1276)
├── Drivers/                    # en-têtes CMSIS vendorisés (STM32L0)
├── startup/                     # linker script + démarrage (STM32L052K8Tx)
└── Makefile                      # compilation arm-none-eabi-gcc
tests/                             # 29 tests, dont 2 validations croisées C/Python
docs/
├── hardware.md                    # nomenclature + brochage + calcul H2
├── pcb.md
└── ground-station.md                # matériel récepteur + récupération
.github/workflows/build.yml          # CI : tests + compilation firmware
```

## Installation et usage

```bash
pip install -r sim/requirements.txt
pytest tests/ -v                    # 24 tests

cd firmware
make                                 # produit build/hab_payload.elf
```

## Licence

MIT pour le code original — voir `LICENSE`. Fichiers CMSIS vendorisés sous
Apache 2.0 — voir `THIRD_PARTY_LICENSES.md`.
