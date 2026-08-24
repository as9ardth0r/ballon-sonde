#include "stm32l0xx.h"
#include "clock.h"

/**
 * Utilise HSI16 (oscillateur RC interne 16 MHz) directement comme
 * SYSCLK, sans PLL ni cristal externe. Choix délibéré pour cette charge
 * utile : les tâches (lecture capteurs, GPS, LoRa) ne sont pas
 * exigeantes en fréquence, et se passer d'un cristal externe simplifie
 * le PCB et réduit la consommation au repos — pertinent pour un vol de
 * plusieurs heures sur pile.
 */
void clock_init_hsi16(void) {
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0) { }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) { }

    SystemCoreClock = 16000000U;
}
