/**
 * ============================================================================
 * @file    delay.h
 * @brief   基于 SysTick 的微秒/毫秒级延时函数声明
 * ============================================================================
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

void delay_init(void);           /* 初始化 SysTick，使用 72MHz 内核时钟 */
void delay_us(uint32_t us);      /* 微秒级延时（阻塞） */
void delay_ms(uint32_t ms);      /* 毫秒级延时（阻塞） */

#endif /* __DELAY_H */
