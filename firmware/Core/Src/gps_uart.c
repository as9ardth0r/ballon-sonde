#include "gps_uart.h"
#include "stm32l0xx.h"

#define LINE_BUF_SIZE 96
static char line_buf[LINE_BUF_SIZE];
static size_t line_len = 0;

void gps_uart_init(uint32_t baudrate) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* PA2=TX, PA3=RX, AF4 = USART2 */
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= (2UL << GPIO_MODER_MODE2_Pos) | (2UL << GPIO_MODER_MODE3_Pos);
    GPIOA->AFR[0] |= (4UL << GPIO_AFRL_AFSEL2_Pos) | (4UL << GPIO_AFRL_AFSEL3_Pos);

    /* USART "nouvelle génération" (registres ISR/TDR/RDR) : avec
     * suréchantillonnage x16 par défaut (OVER8=0), BRR se calcule
     * directement, pas de découpage mantisse/fraction comme sur
     * l'ancien périphérique du STM32F405 (projet nanodrone). */
    USART2->BRR = (uint16_t)(16000000UL / baudrate);
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART2->CR1 |= USART_CR1_UE;

    line_len = 0;
}

void gps_send_ubx_frame(const uint8_t *frame, size_t len) {
    for (size_t i = 0; i < len; i++) {
        while ((USART2->ISR & USART_ISR_TXE) == 0) { }
        USART2->TDR = frame[i];
    }
}

bool gps_uart_poll_line(char *line_out, size_t max_len) {
    if ((USART2->ISR & USART_ISR_RXNE) == 0) return false;

    char c = (char)(USART2->RDR & 0xFF);

    if (c == '\n' || c == '\r') {
        if (line_len == 0) return false;
        size_t copy_len = (line_len < max_len - 1) ? line_len : max_len - 1;
        for (size_t i = 0; i < copy_len; i++) { line_out[i] = line_buf[i]; }
        line_out[copy_len] = '\0';
        line_len = 0;
        return true;
    }

    if (line_len < LINE_BUF_SIZE - 1) {
        line_buf[line_len++] = c;
    } else {
        line_len = 0; /* ligne trop longue : réinitialise plutôt que déborder */
    }
    return false;
}
