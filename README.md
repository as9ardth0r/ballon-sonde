# ballon-sonde

### Stratospheric weather balloon payload — STM32L0, BME280, GPS, LoRa

Measures temperature/humidity/pressure and GPS position throughout the
ascent, transmitting continuously over LoRa (no need to recover the
payload to get the data), designed for a multi-hour flight on batteries
in stratospheric cold.

## What's real and verified (29 tests)

| Component | Verified how |
|---|---|
| **BME280 compensation** (temperature/pressure/humidity) | Bosch formulas (datasheet §4.2.3). Tested against the International Standard Atmosphere (5000 m ↔ 54048 Pa, ±150 m tolerance) |
| **NMEA GGA parser** | Validated against the canonical NMEA standard example, checksum recomputed by the code rather than copied |
| **UBX-CFG-NAV5 frame** (GPS flight mode) | Structure and Fletcher checksum verified |
| **LoRa telemetry protocol** | C ↔ Python encoding/decoding compared via `ctypes` — **a real bug caught along the way**: `float` (single precision) was truncating a GPS latitude at the 6th decimal, fixed to `double` throughout |
| **Ground station** (`ground_station.py`) | CSV decoding/logging tested (valid frames, radio noise, corrupted checksum, full CSV round-trip) |
| **STM32L052 firmware** | Actually compiles and **links** with `arm-none-eabi-gcc` (46.5 KB / 64 KB of flash) against official ARM/ST CMSIS headers |
| **I2C, UART, SPI drivers** | Written specifically for the STM32L0's "new generation" I2Cv2/USART peripheral — different from the nanodrone project's (STM32F405, I2Cv1), not a hastily adapted copy |

## What is NOT verified

- **BME280 and LoRa (firmware)**: publicly documented registers, drivers written in full (unlike the nanodrone's VL53L1X), but not tested on real sensor/module hardware — no hardware available in this development environment.
- **The firmware-side BME280 compensation formula** (`bme280.c`) follows the same structure as `bme280_compensate.py`, but hasn't been numerically cross-checked via `ctypes` the way `telemetry.c` was — worth doing if a stronger guarantee is needed.
- **No real flight, no bench measurement.**

## Bill of materials, PCB, and hydrogen calculation

- **[docs/hardware.md](docs/hardware.md)** — precise bill of materials (part number, weight, role), pinout, points specific to a stratospheric flight (GPS limits, BME280 range, batteries), and **the hydrogen quantity calculation** (~0.6-0.7 m³ for this payload — see the full calculation and safety/precision margins in the document).
- **[docs/pcb.md](docs/pcb.md)** — lighter treatment than the nanodrone project (no high currents); the real concern here is thermal insulation, not routing.
- **[docs/ground-station.md](docs/ground-station.md)** — receiver hardware, in-flight tracking, recovery logistics.

## Repository structure

```
sim/hab_sim/
├── bme280_compensate.py   # temperature/pressure/humidity compensation
├── nmea.py                 # GGA parser
├── ubx.py                   # GPS config frame (flight mode)
├── telemetry.py               # LoRa protocol
└── ground_station.py            # ground-side decoding + logging
firmware/
├── Core/Inc, Core/Src        # real drivers (I2Cv2, UART, SPI, BME280, SX1276)
├── Drivers/                    # vendored CMSIS headers (STM32L0)
├── startup/                     # linker script + startup (STM32L052K8Tx)
└── Makefile                      # arm-none-eabi-gcc build
tests/                             # 29 tests, including 2 C/Python cross-validations
docs/
├── hardware.md                    # bill of materials + pinout + H2 calculation
├── pcb.md
└── ground-station.md                # receiver hardware + recovery
.github/workflows/build.yml          # CI: tests + firmware compilation
```

## Installation and usage

```bash
pip install -r sim/requirements.txt
pytest tests/ -v                    # 24 tests

cd firmware
make                                 # produces build/hab_payload.elf
```

## License

MIT for original code — see `LICENSE`. Vendored CMSIS files under
Apache 2.0 — see `THIRD_PARTY_LICENSES.md`.
