/**
 * ============================================================================
 * @file    delay.c
 * @brief   基于 SysTick 的延时实现
 *
 * 系统时钟配置为 64MHz（内部 HSI 8MHz/2 × PLL16），SysTick 使用内核时钟源，
 * 因此每计数一次耗时 1/64MHz ≈ 15.6ns。
 * 不依赖外部晶振，兼容任意 STM32F103 最小系统板。
 * ============================================================================
 */

#include "delay.h"

/* 每微秒对应的 SysTick 计数个数（64MHz 下 = 64） */
#define TICKS_PER_US    (64U)

/* SysTick 是 24 位递减计数器，最大计数 0xFFFFFF，单次延时不能超过该值 */
#define MAX_DELAY_US    (0xFFFFFFUL / TICKS_PER_US)

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
 * @brief 微秒级延时（带 24 位计数器溢出保护）
 * @param us 延时微秒数
 * @note  超过 SysTick 24 位计数器上限时，自动分块延时，避免溢出
 */
void delay_us(uint32_t us)
{
    while (us > 0) {
        uint32_t ticks;

        /* 24 位计数器上限保护：单次超过 0xFFFFFF 时分块延时 */
        if (us > MAX_DELAY_US) {
            ticks = MAX_DELAY_US * TICKS_PER_US;
        } else {
            ticks = us * TICKS_PER_US;
        }

        SysTick->LOAD = ticks;
        SysTick->VAL  = 0;
        SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_CLKSOURCE;

        /* 等待计数到 0（COUNTFLAG 置位） */
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG)) {
            ;
        }

        SysTick->CTRL = 0;      /* 关闭 SysTick */

        us -= ticks / TICKS_PER_US;   /* 减去本次实际延时的微秒数 */
    }
}

/**
 * @brief 毫秒级延时
 * @param ms 延时毫秒数
 * @note  分块调用 delay_us 以支持任意时长
 */
void delay_ms(uint32_t ms)
{
    while (ms--) {
        delay_us(1000);
    }
}
