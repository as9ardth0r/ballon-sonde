# Matériel — nomenclature détaillée

Configuration : STM32L0 (ultra basse consommation), BME280 (température/
humidité/pression), GPS u-blox avec mode vol configuré, LoRa 868 MHz (EU)
pour la télémétrie descendante. Références réelles, vérifiées au moment de
la rédaction — les prix évoluent, à revérifier avant achat.

## Nomenclature (BOM)

| # | Composant | Référence précise | Caractéristiques clés | Poids approx. | Rôle |
|---|---|---|---|---|---|
| 1 | Microcontrôleur | **STM32L052K8T6** (LQFP32) | Cortex-M0+, 32 MHz max (utilisé ici à 16 MHz HSI), 64 Ko Flash, 8 Ko RAM, consommation µA en veille | ~0,15 g (puce seule) | Calcul, cadencement des mesures/envois |
| 2 | Capteur environnemental | **Bosch BME280** (breakout, ex. GY-BME280) | Température/humidité/pression, I²C, plage **-40 à +85 °C** — voir limite ci-dessous | ~1-2 g | Mesures scientifiques |
| 3 | GPS | **u-blox MAX-M10S** (breakout, ex. SparkFun GNSS MAX-M10S) | Multi-constellation (GPS/Galileo/GLONASS/BeiDou), 25 mW en suivi continu, compatible vol haute altitude — voir note COCOM | ~2-5 g (module) + antenne | Position + altitude au-delà de la portée du baromètre |
| 4 | Antenne GPS | Céramique active ou passive selon le module | À orienter vers le ciel, dégagée de tout écran métallique | ~1-3 g | Réception satellite |
| 5 | Module LoRa | **RFM95W (puce SX1276)**, 868 MHz EU | SPI, jusqu'à ~20 dBm, portée air-sol de plusieurs dizaines de km en visibilité directe à haute altitude | ~2-3 g | Télémétrie descendante |
| 6 | Antenne LoRa | Fil quart d'onde ~8,2 cm (868 MHz) ou antenne fouet dédiée | À l'extérieur du boîtier, verticale | ~1 g | Émission LoRa |
| 7 | Piles | **2× Energizer Ultimate Lithium AA**, PAS de LiPo | Lithium primaire, fiable jusqu'à **-40 °C** (contrairement au LiPo qui perd sa capacité et peut être endommagé par le froid stratosphérique) | ~29 g (2×14,5 g) | Alimentation — tout le vol dure plusieurs heures dans le froid |
| 8 | Régulateur 3.3V | **MCP1700-3302E** ou équivalent LDO basse consommation | Faible courant de repos — important en usage batterie longue durée | <0,1 g | Alim MCU/capteurs |
| 9 | Boîtier charge utile | Boîte en polystyrène expansé, ~10×10×10 cm | Léger, isolant thermique (protège l'électronique du froid extérieur) | ~30-40 g | Protection mécanique/thermique |
| 10 | Parachute | ~30-45 cm de diamètre, adapté à une charge <300 g | Descente contrôlée après éclatement | ~20-25 g | Sécurité à la retombée |
| 11 | Cordage | Ligne de nylon/kevlar, ~2-3 m entre ballon/parachute/charge | Chaîne de vol standard : ballon — parachute — charge utile | ~10 g | Assemblage |
| 12 | Ballon | Latex, **~350 g** (ex. Hwoyee 350) | Taille adaptée à ce poids de charge — voir calcul de gaz plus bas | 350 g | Portance |
| 13 | Connecteur programmation | Header SWD 4 broches | Flash/debug via ST-Link | négligeable | Programmation |
| 14 | Condensateurs découplage | 100 nF ×4-6 + 10 µF bulk | Standard | négligeable | Stabilité alimentation |
| 15 | Résistances pull-up I²C ×2 | 4,7 kΩ | Bus BME280 | négligeable | Bus I²C |

**Masse totale électronique + boîtier + parachute + cordage : ~130-150 g**
(hors ballon et gaz). C'est la masse utilisée dans le calcul de gaz ci-dessous.

## Points de vigilance spécifiques à un vol stratosphérique

- **GPS et limite COCOM** — les modules GPS grand public coupent leur sortie
  au-delà de 18 km d'altitude **ET** 515 m/s de vitesse simultanément
  (restriction export liée à un usage missile). Un ballon reste très en
  dessous de 515 m/s, donc **cette limite ne s'applique pas** ici — confirmé
  par la documentation UKHAS (communauté ballon-sonde amateur) pour les
  familles u-blox 6/7/8/M10. Le firmware (`ubx.c`) configure en plus le mode
  dynamique "Airborne <1g" au démarrage, ce qui est un réglage différent
  (améliore la qualité du fix en ascension/chute rapide) — les deux sujets
  sont parfois confondus, ils ne se règlent pas de la même façon.
- **BME280 hors spec au point le plus froid** — la tropopause (10-15 km
  environ, selon la saison/latitude) descend couramment à -55/-60 °C, sous
  la limite -40 °C de la datasheet BME280. Les lectures dans cette tranche
  d'altitude doivent être prises avec prudence (précision non garantie hors
  plage), même si le capteur continue généralement à répondre.
- **Piles au lithium, pas de LiPo** — voir ligne 7 du tableau. C'est le
  piège le plus classique de la communauté ballon amateur : un LiPo qui
  fonctionne parfaitement au sol peut chuter à une fraction de sa capacité,
  voire s'endommager, au froid stratosphérique.
- **Coordination espace aérien** — un lâcher de ballon haute altitude
  nécessite en général une notification préalable à la DGAC en France. Je ne
  suis pas juriste ; à vérifier directement avant tout vol réel.

## Combien d'hydrogène ?

Calcul basé sur la nomenclature ci-dessus, à titre d'ordre de grandeur — pas
un substitut à un vrai calculateur de vol avant un lancement réel (voir plus
bas).

**Pouvoir ascensionnel de l'hydrogène** : la portance vient de la différence
de densité entre l'air (~1,2-1,225 kg/m³ au niveau de la mer) et l'hydrogène
(~0,090 kg/m³), soit environ **1,1 à 1,2 kg de portance par m³** de gaz
(valeur qui varie légèrement avec la température ambiante réelle — utilisé
ici : 1,1 kg/m³, plutôt conservateur).

**Masse fixe à soulever** : charge utile (~150 g) + ballon (~350 g) = **500 g**

**Portance libre ("free lift")** : l'excédent de portance au-delà du
strict équilibre détermine la vitesse d'ascension. Un ratio d'environ 40 %
de la masse fixe est cohérent avec des vols amateurs documentés visant une
ascension raisonnable (~5 m/s) — soit ici environ **200 g** de portance
libre.

**Calcul** :
```
Portance totale nécessaire = masse fixe + portance libre
                            = 500 g + 200 g = 700 g

Volume d'hydrogène = 700 g / 1,1 kg/m³ ≈ 0,64 m³

Masse d'hydrogène (le gaz lui-même, pas la portance) :
                    0,64 m³ × 0,090 kg/m³ ≈ 58 g de H₂
```

**→ Environ 0,6 à 0,7 m³ d'hydrogène** pour ce montage (masse de gaz elle-même
: quelques dizaines de grammes seulement — l'essentiel du "poids" du calcul
est la portance déplacée, pas la masse du gaz). C'est une quantité modeste,
largement contenue dans une petite bouteille industrielle standard.

**⚠️ Sécurité et précision — deux réserves importantes** :

1. **L'hydrogène est inflammable/explosif au contact de l'air.** Beaucoup de
   groupes amateurs utilisent de l'hélium précisément pour éviter ce risque,
   malgré la portance légèrement supérieure et le coût plus faible de
   l'hydrogène. Si hydrogène : remplissage en extérieur, aucune flamme/
   étincelle à proximité, raccords et détendeur adaptés au gaz, idéalement
   fourni par un professionnel avec fiche de sécurité — je ne suis pas
   formé à la manipulation de gaz industriels, ceci n'est pas une procédure
   de sécurité complète.
2. **Ce calcul est un ordre de grandeur**, pas un chiffre à utiliser tel
   quel avant un vol réel. L'altitude d'éclatement précise dépend du
   diamètre d'éclatement spécifique du ballon (donnée du fabricant, pas
   générique), qui n'est pas incluse ici. Avant un lancement réel, utiliser
   un vrai calculateur (le calculateur CUSF/Habhub, standard dans la
   communauté ballon amateur, ou la fiche de remplissage du fabricant du
   ballon) avec les données exactes du ballon choisi.

## Plan de brochage (STM32L052K8T6, LQFP32)

| Fonction | Broche(s) | AF | Notes |
|---|---|---|---|
| I2C1 (BME280) | PB6 (SCL), PB7 (SDA) | AF1 | 100 kHz |
| USART2 (GPS) | PA2 (TX), PA3 (RX) | AF4 | 9600 bauds |
| SPI1 (LoRa) | PA5 (SCK), PA6 (MISO), PA7 (MOSI) | AF0 | |
| NSS LoRa | PA4 | GPIO sortie | Chip select logiciel |
| SWDIO / SWCLK | PA13 / PA14 | AF0 | Programmation/debug |

## Ce qui reste ouvert

- **PCB** : voir [docs/pcb.md](pcb.md) — traitement plus léger que pour le
  projet nanodrone, ce montage a moins de contraintes mécaniques (pas de
  moteurs, pas de courants forts).
- **ADC batterie** : `frame.battery_mv` est câblé dans le protocole mais pas
  encore lu (pont diviseur + ADC non implémentés dans ce squelette).
- **Réception au sol** : ce dépôt couvre l'émetteur (charge utile), pas le
  récepteur LoRa au sol — un module RFM95W + ordinateur/Raspberry Pi
  suffirait, non traité ici.
