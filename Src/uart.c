/**
 * ============================================================================
 * @file    uart.c
 * @brief   串口驱动实现（寄存器级，无中断，阻塞式收发）
 *
 * 时钟配置：系统时钟 72MHz
 *   - USART1 挂在 APB2（72MHz）
 *   - USART2 挂在 APB1（36MHz）
 * 波特率 115200：
 *   - USART1 BRR = 72000000 / 115200 = 625.0  -> 0x2710
 *   - USART2 BRR = 36000000 / 115200 = 312.5  -> 0x1388
 * ============================================================================
 */

#include "uart.h"

/* APB2 / APB1 总线时钟（Hz），与 system_clock 配置保持一致 */
#define PCLK2_HZ        72000000UL
#define PCLK1_HZ        36000000UL

/**
 * @brief 计算 USART BRR 寄存器值
 * @param pclk 总线时钟（Hz）
 * @param baud 目标波特率
 * @return BRR 寄存器值（高 12 位整数部分，低 4 位小数部分）
 */
static uint16_t uart_calc_brr(uint32_t pclk, uint32_t baud)
{
    uint32_t mantissa, fraction;
    mantissa = pclk / baud;
    fraction = ((pclk % baud) * 16 + baud / 2) / baud;
    return (uint16_t)((mantissa << 4) | fraction);
}

/**
 * @brief 配置某个 USART 的 TX/RX 引脚
 */
static void uart_gpio_config(USART_TypeDef *uart)
{
    if (uart == USART1) {
        /* PA9 = USART1_TX（复用推挽），PA10 = USART1_RX（浮空输入） */
        RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

        GPIOA->CRH &= ~(0xFFUL << GPIO_CRH_SHIFT(8));   /* 清 PA8/PA9 配置 */
        GPIOA->CRH |= (GPIO_CNF_AF_PP | GPIO_MODE_OUT_50M) << GPIO_CRH_SHIFT(8);
        GPIOA->CRH |= (GPIO_CNF_IN_FLOAT) << GPIO_CRH_SHIFT(9);
    } else if (uart == USART2) {
        /* PA2 = USART2_TX，PA3 = USART2_RX */
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

        GPIOA->CRL &= ~(0xFFUL << GPIO_PIN_SHIFT(2));   /* 清 PA2/PA3 配置 */
        GPIOA->CRL |= (GPIO_CNF_AF_PP | GPIO_MODE_OUT_50M) << GPIO_PIN_SHIFT(2);
        GPIOA->CRL |= (GPIO_CNF_IN_FLOAT) << GPIO_PIN_SHIFT(3);
    }
}

/**
 * @brief 初始化 USART（8N1，无流控）
 */
static void uart_config(USART_TypeDef *uart, uint32_t pclk, uint32_t baud)
{
    uart_gpio_config(uart);

    uart->BRR = uart_calc_brr(pclk, baud);
    uart->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/**
 * @brief 初始化两个串口
 *   - USART1：调试输出（PA9/PA10）
 *   - USART2：舵机通信（PA2/PA3）
 */
void uart_init(void)
{
    uart_config(USART1, PCLK2_HZ, UART_BAUD_115200);
    uart_config(USART2, PCLK1_HZ, UART_BAUD_115200);
}

/**
 * @brief 阻塞发送单个字节
 */
void uart_send_byte(USART_TypeDef *uart, uint8_t data)
{
    /* 等待发送数据寄存器空 */
    while (!(uart->SR & USART_SR_TXE)) {
        ;
    }
    uart->DR = data;
}

/**
 * @brief 阻塞发送字节数组
 */
void uart_send_bytes(USART_TypeDef *uart, const uint8_t *buf, uint16_t len)
{
    while (len--) {
        uart_send_byte(uart, *buf++);
    }
}

/**
 * @brief 发送字符串（以 '\0' 结尾）
 */
void uart_send_string(USART_TypeDef *uart, const char *str)
{
    while (*str) {
        uart_send_byte(uart, (uint8_t)(*str++));
    }
}
