# Station sol — matériel et logistique de récupération

## Récepteur : deux options réelles

**Option simple (recommandée pour démarrer)** : une carte type **Adafruit
Feather 32u4 LoRa** ou **Heltec WiFi LoRa 32** (ESP32 + SX1276/SX1278
intégrés, USB natif) — flashée avec un croquis minimal qui relaie chaque
paquet LoRa reçu sur le port série USB en texte brut. `ground_station.py`
lit ensuite ce port série directement. Pas besoin de redévelopper un
firmware complet : ces cartes ont des bibliothèques LoRa Arduino toutes
faites (ex. bibliothèque `RadioHead` ou `LoRa` de Sandeep Mistry),
largement suffisantes pour de la simple réception.

**Option réutilisant ce dépôt** : un second module RFM95W + n'importe quel
petit microcontrôleur, en reprenant `lora_sx1276.c` de ce dépôt — il
faudrait ajouter une fonction `lora_receive()` (mode continuous RX plutôt
que TX, lecture du FIFO sur interruption DIO0) qui n'existe pas encore ici.
Plus de travail, mais cohérent avec le reste du dépôt si tu préfères rester
sur du STM32.

Dans les deux cas : **même réglage radio des deux côtés** (868 MHz, BW
125 kHz, SF9, CR 4/5 — voir la correction apportée à `lora_sx1276.c`),
sinon le récepteur n'entend rien.

## Logiciel de réception

`sim/hab_sim/ground_station.py` — lit le port série, décode avec le même
code que celui déjà validé contre le firmware (`hab_sim.telemetry`),
journalise en CSV, affiche un état en direct :

```bash
pip install pyserial
python -m hab_sim.ground_station --port /dev/ttyUSB0 --baud 9600 --log vol_2026-08-24.csv
```

Le CSV contient position, altitude, température, humidité, pression et
tension batterie à chaque réception — **les données scientifiques sont
donc déjà récupérées en temps réel**, avant même de retrouver la charge
utile physique.

## Suivi pendant le vol et récupération physique

- **Avant le lancement** : prédire la trajectoire et le point d'atterrissage
  avec le calculateur CUSF/Habhub (le même outil déjà recommandé pour le
  calcul de gaz) à partir des prévisions vent en altitude du jour.
- **Pendant le vol** : la station sol peut être mobile (ordinateur portable
  + batterie + antenne directive type Yagi 868 MHz dans une voiture) pour
  se rapprocher de la trajectoire prévue au fur et à mesure — utile car la
  dérive horizontale peut atteindre plusieurs dizaines de km sur un vol de
  2-3h.
- **Portée** : SF9 donne une bonne marge de liaison, mais la portée réelle
  dépend énormément de la ligne de vue (une charge utile à 20 km
  d'altitude a une ligne de vue radio de plusieurs centaines de km en
  théorie ; un récepteur au sol dans une vallée ou entouré de bâtiments
  fera bien pire).
- **Retrouver la charge au sol** : une fois dans la zone d'atterrissage
  estimée, les dernières positions GPS reçues avant la perte de signal (le
  signal LoRa décroche généralement bien avant l'atterrissage si le
  récepteur est loin) donnent une zone de recherche. Le GPS embarqué
  continue en principe d'émettre au sol tant que la batterie tient — utile
  si on peut se rapprocher suffisamment après coup.

## Ce qui n'est pas couvert ici

- Pas de firmware récepteur complet fourni (voir les deux options
  matérielles ci-dessus).
- Pas d'intégration avec les plateformes de tracking communautaires
  (SondeHub, Habhub) — leur format de trame standard (proche
  UKHAS/RTTY-style) est différent du protocole simplifié utilisé ici. Une
  passerelle de traduction serait possible mais n'est pas construite dans
  ce dépôt.
