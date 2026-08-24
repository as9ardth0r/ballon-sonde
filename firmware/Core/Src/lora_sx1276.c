#include "lora_sx1276.h"
#include "spi1_bus.h"
#include "gpio.h"

/* Registres SX1276 (datasheet Semtech DS_SX1276-7-8-9_W_APP_V7) */
#define REG_FIFO           0x00U
#define REG_OP_MODE        0x01U
#define REG_FRF_MSB        0x06U
#define REG_FRF_MID        0x07U
#define REG_FRF_LSB        0x08U
#define REG_PA_CONFIG      0x09U
#define REG_FIFO_ADDR_PTR  0x0DU
#define REG_FIFO_TX_BASE   0x0EU
#define REG_IRQ_FLAGS      0x12U
#define REG_MODEM_CONFIG1  0x1DU
#define REG_MODEM_CONFIG2  0x1EU
#define REG_PREAMBLE_MSB   0x20U
#define REG_PREAMBLE_LSB   0x21U
#define REG_PAYLOAD_LENGTH 0x22U
#define REG_VERSION        0x42U

#define MODE_LONG_RANGE    0x80U /* bit7 RegOpMode : active le mode LoRa */
#define MODE_SLEEP         0x00U
#define MODE_STDBY         0x01U
#define MODE_TX            0x03U

#define IRQ_TX_DONE        0x08U

#define SX1276_VERSION_EXPECTED 0x12U

/* Broche NSS (chip select) du module — GPIO dédié, voir docs/hardware.md */
#define NSS_PIN GPIO_PIN('A', 4)

static void nss_low(void)  { gpio_write(NSS_PIN, false); }
static void nss_high(void) { gpio_write(NSS_PIN, true); }

static void write_reg(uint8_t reg, uint8_t value) {
    nss_low();
    spi1_transfer(reg | 0x80U); /* bit7=1 : écriture */
    spi1_transfer(value);
    nss_high();
}

static uint8_t read_reg(uint8_t reg) {
    nss_low();
    spi1_transfer(reg & 0x7FU); /* bit7=0 : lecture */
    uint8_t value = spi1_transfer(0x00U);
    nss_high();
    return value;
}

bool lora_init(void) {
    gpio_init_output(NSS_PIN);
    nss_high();
    spi1_init();

    write_reg(REG_OP_MODE, MODE_LONG_RANGE | MODE_SLEEP);

    uint8_t version = read_reg(REG_VERSION);
    if (version != SX1276_VERSION_EXPECTED) return false;

    write_reg(REG_OP_MODE, MODE_LONG_RANGE | MODE_STDBY);

    /* 868.0 MHz (bande ISM EU, cristal de référence 32 MHz) :
     * Frf = 868 000 000 * 2^19 / 32 000 000 = 14 221 312 = 0xD90000 —
     * valeur vérifiée par calcul, pas recopiée d'un exemple. */
    write_reg(REG_FRF_MSB, 0xD9U);
    write_reg(REG_FRF_MID, 0x00U);
    write_reg(REG_FRF_LSB, 0x00U);

    /* Bande passante 125 kHz, coding rate 4/5, en-tête explicite (0x72).
     * Spreading factor 9 plutôt que 7 : le SF7 initial privilégiait le
     * débit, alors que l'usage réel (retrouver un ballon qui peut dériver
     * à des dizaines de km) demande la sensibilité maximale, pas la
     * vitesse — un paquet toutes les quelques secondes suffit largement
     * pour du tracking. SF9 est un compromis courant dans la communauté
     * ballon-sonde amateur : nette amélioration de portée par rapport à
     * SF7, sans tomber dans les temps de vol paquet de SF12 (secondes
     * par paquet, inutilement lent pour ce débit de données). CRC activé
     * (0x94 = SF9 + CRC).*/
    write_reg(REG_MODEM_CONFIG1, 0x72U);
    write_reg(REG_MODEM_CONFIG2, 0x94U);

    write_reg(REG_PREAMBLE_MSB, 0x00U);
    write_reg(REG_PREAMBLE_LSB, 0x08U);

    /* PA_BOOST, puissance ~17 dBm — voir docs/hardware.md pour le
     * respect du rapport cyclique ETSI sur la bande 868 MHz */
    write_reg(REG_PA_CONFIG, 0x8FU);

    return true;
}

bool lora_send(const uint8_t *data, size_t len) {
    if (len > 255) return false;

    write_reg(REG_OP_MODE, MODE_LONG_RANGE | MODE_STDBY);
    write_reg(REG_FIFO_TX_BASE, 0x00U);
    write_reg(REG_FIFO_ADDR_PTR, 0x00U);

    nss_low();
    spi1_transfer(REG_FIFO | 0x80U);
    for (size_t i = 0; i < len; i++) { spi1_transfer(data[i]); }
    nss_high();

    write_reg(REG_PAYLOAD_LENGTH, (uint8_t)len);
    write_reg(REG_OP_MODE, MODE_LONG_RANGE | MODE_TX);

    uint32_t timeout = 2000000U; /* borne large : pas de mesure de temps précise ici */
    while ((read_reg(REG_IRQ_FLAGS) & IRQ_TX_DONE) == 0) {
        if (--timeout == 0) return false;
    }
    write_reg(REG_IRQ_FLAGS, IRQ_TX_DONE); /* acquitte le flag (write-1-to-clear) */
    return true;
}
