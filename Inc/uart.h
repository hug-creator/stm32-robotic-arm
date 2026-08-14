/**
 * ============================================================================
 * @file    uart.h
 * @brief   串口驱动声明（USART1 调试 + USART2 舵机通信）
 * ============================================================================
 */

#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

#define UART_BAUD_115200    115200U

/* 舵机通信串口（USART2：PA2=TX, PA3=RX） */
#define SERVO_UART          USART2
/* 调试串口（USART1：PA9=TX, PA10=RX） */
#define DEBUG_UART          USART1

void uart_init(void);                                   /* 初始化两个串口 115200 */
void uart_send_byte(USART_TypeDef *uart, uint8_t data); /* 发送单字节（阻塞） */
void uart_send_bytes(USART_TypeDef *uart, const uint8_t *buf, uint16_t len);
void uart_send_string(USART_TypeDef *uart, const char *str);

#endif /* __UART_H */
