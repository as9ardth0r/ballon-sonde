# PCB — recommandations

Traitement plus léger que pour le projet nanodrone : pas de moteurs, pas de
courants forts, pas de contrainte de poids extrême. Les points qui comptent
vraiment ici sont différents.

## Stack-up

**2 couches suffisent largement.** Aucune piste ne porte plus de quelques
dizaines de mA (I2C, UART, SPI, un module LoRa en émission pointe autour de
100-120 mA en crête pendant quelques centaines de ms seulement). Pas de
calcul IPC-2221 nécessaire à ce niveau de courant — les largeurs de piste
standard (10-15 mil) sont confortables partout.

## Le vrai sujet : le froid, pas le routage

Le point de conception qui compte le plus ici n'est pas électrique, c'est
thermique. À -55/-60°C (tropopause), sans précaution :
- Les piles perdent en performance (d'où le choix Energizer Ultimate
  Lithium plutôt que LiPo — voir docs/hardware.md)
- Le BME280 sort de sa plage garantie
- Les joints de soudure et connecteurs bon marché peuvent devenir cassants

**Recommandations concrètes** :
- Placer l'électronique (PCB, piles) **au centre** du boîtier polystyrène,
  pas contre une paroi extérieure — la masse du boîtier + l'air immobile à
  l'intérieur agissent comme isolant.
- Le BME280 doit rester en contact avec l'air extérieur (sinon il mesure la
  température de l'intérieur du boîtier, pas l'atmosphère) — un petit trou
  ou une fente dans le boîtier, avec le capteur monté juste derrière,
  suffit généralement. Ce compromis (isoler l'électronique / exposer le
  capteur) est le point de tension principal du placement mécanique.
- L'antenne GPS doit voir le ciel — face supérieure du boîtier, pas
  entourée de métal (piles, câblage groupé) qui masquerait le signal.

## Emplacements réservés

- **Antenne LoRa** : sortie verticale hors du boîtier, éloignée du plan de
  masse du PCB pour ne pas dégrader le rayonnement.
- **Header SWD** : accessible sans démonter tout le boîtier, pour re-flasher
  entre deux vols de test.
- **Découplage** : 100 nF au plus près de chaque broche VDD du STM32L052 et
  du BME280 ; le 10 µF bulk près du régulateur.

## Ce qui reste à faire

Routage réel (KiCad), empreintes des composants, et un test de continuité
avant tout vol — rien de plus exotique que pour un projet électronique
embarqué classique une fois le placement thermique pris en compte.
