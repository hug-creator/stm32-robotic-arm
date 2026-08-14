/**
 * ============================================================================
 * @file    delay.c
 * @brief   基于 SysTick 的延时实现
 *
 * 系统时钟配置为 72MHz（HSE 8MHz * PLL9），SysTick 使用内核时钟源，
 * 因此每计数一次耗时 1/72MHz ≈ 13.9ns。
 * ============================================================================
 */

#include "delay.h"

/* 每微秒对应的 SysTick 计数个数（72MHz 下 = 72） */
#define TICKS_PER_US    (72U)

/**
 * @brief 初始化 SysTick
 * @note  关闭计数器，装载最大值，供后续 delay_us/delay_ms 使用
 */
void delay_init(void)
{
    SysTick->CTRL = 0;          /* 先关闭 SysTick */
    SysTick->LOAD = 0xFFFFFF;   /* 装载最大计数 */
    SysTick->VAL  = 0;          /* 清空当前值 */
}

/**
 * @brief 微秒级延时
 * @param us 延时微秒数
 */
void delay_us(uint32_t us)
{
    uint32_t ticks = us * TICKS_PER_US;

    SysTick->LOAD = ticks;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_CLKSOURCE;

    /* 等待计数到 0（COUNTFLAG 置位） */
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG)) {
        ;
    }

    SysTick->CTRL = 0;          /* 关闭 SysTick */
}

/**
 * @brief 毫秒级延时
 * @param ms 延时毫秒数
 * @note  对超长延时，分块调用 delay_us 以避免 24 位计数器溢出
 */
void delay_ms(uint32_t ms)
{
    while (ms--) {
        delay_us(1000);
    }
}
