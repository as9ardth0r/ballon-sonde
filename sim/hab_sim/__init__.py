"""hab_sim — logique de charge utile pour ballon-sonde stratosphérique :
compensation BME280, parsing NMEA/UBX GPS, protocole de télémétrie LoRa.
Miroir Python du firmware C (firmware/Core/Src/), validé par comparaison
numérique croisée — voir tests/.
"""

__version__ = "0.1.0"
