/**
 * ============================================================================
 * @file    system_stm32f10x.c
 * @brief   SystemInit 实现
 *
 * 启动文件在复位后、跳转 main() 之前会调用 SystemInit()。
 * 本项目的时钟配置（72MHz PLL）放在 main() 的 system_clock_config() 中
 * 逐步完成，便于学习和理解，因此 SystemInit 这里保持空实现。
 * ============================================================================
 */

#include "system_stm32f10x.h"

void SystemInit(void)
{
    /* 时钟配置在 main() 的 system_clock_config() 中完成 */
}
