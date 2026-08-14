/**
 * ============================================================================
 * @file    stm32f10x.h
 * @brief   STM32F103 寄存器级（裸机）寄存器定义
 *
 * 本文件仅定义本项目用到的外设寄存器，通过结构体 + 宏的方式直接访问
 * 内存映射地址，不依赖任何厂商库，代码完全自包含。
 *
 * 参考：STM32F103 参考手册 RM0008（外设地址与位定义）
 * ============================================================================
 */

#ifndef __STM32F10X_H
#define __STM32F10X_H

#include <stdint.h>

/* ============================ 外设基地址 ============================ */

#define FLASH_BASE          0x40022000UL   /* FLASH 控制器            */
#define RCC_BASE            0x40021000UL   /* 复位与时钟控制 RCC      */
#define GPIOA_BASE          0x40010800UL   /* GPIOA                   */
#define GPIOB_BASE          0x40010C00UL   /* GPIOB                   */
#define GPIOC_BASE          0x40011000UL   /* GPIOC                   */
#define AFIO_BASE           0x40010000UL   /* 复用功能 IO AFIO         */

#define TIM2_BASE           0x40000000UL   /* 通用定时器 TIM2         */
#define TIM3_BASE           0x40000400UL   /* 通用定时器 TIM3         */

#define USART1_BASE         0x40013800UL   /* 串口 1（APB2）          */
#define USART2_BASE         0x40004400UL   /* 串口 2（APB1）          */

/* ============================ RCC ============================ */

typedef struct {
    uint32_t CR;          /* 0x00 时钟控制寄存器           */
    uint32_t CFGR;        /* 0x04 时钟配置寄存器           */
    uint32_t CIR;         /* 0x08 时钟中断寄存器           */
    uint32_t APB2RSTR;    /* 0x0C APB2 外设复位寄存器      */
    uint32_t APB1RSTR;    /* 0x10 APB1 外设复位寄存器      */
    uint32_t AHBENR;      /* 0x14 AHB 外设时钟使能寄存器   */
    uint32_t APB2ENR;     /* 0x18 APB2 外设时钟使能寄存器  */
    uint32_t APB1ENR;     /* 0x1C APB1 外设时钟使能寄存器  */
    uint32_t BDCR;        /* 0x20 备份域控制寄存器         */
    uint32_t CSR;         /* 0x24 控制/状态寄存器          */
} RCC_TypeDef;

#define RCC                 ((RCC_TypeDef *)RCC_BASE)

/* RCC_CR 位定义 */
#define RCC_CR_HSION        (1UL << 0)   /* 内部高速时钟使能           */
#define RCC_CR_HSIRDY       (1UL << 1)   /* 内部高速时钟就绪           */
#define RCC_CR_HSEON        (1UL << 16)  /* 外部高速时钟使能           */
#define RCC_CR_HSERDY       (1UL << 17)  /* 外部高速时钟就绪           */
#define RCC_CR_PLLON        (1UL << 24)  /* PLL 使能                   */
#define RCC_CR_PLLRDY       (1UL << 25)  /* PLL 就绪                   */

/* RCC_CFGR 位定义 */
#define RCC_CFGR_SW_HSI     (0UL << 0)   /* 系统时钟选择：HSI          */
#define RCC_CFGR_SW_PLL     (2UL << 0)   /* 系统时钟选择：PLL          */
#define RCC_CFGR_SWS_HSI    (0UL << 2)   /* 系统时钟状态：HSI          */
#define RCC_CFGR_SWS_PLL    (2UL << 2)   /* 系统时钟状态：PLL          */
#define RCC_CFGR_PLLSRC     (1UL << 16)  /* PLL 时钟源 = HSE           */
#define RCC_CFGR_PLLMULL9   (7UL << 18)  /* PLL 倍频系数 = 9           */

/* RCC_APB2ENR 位定义 */
#define RCC_APB2ENR_AFIOEN  (1UL << 0)   /* 复用功能 IO 时钟           */
#define RCC_APB2ENR_IOPAEN  (1UL << 2)   /* GPIOA 时钟                 */
#define RCC_APB2ENR_IOPBEN  (1UL << 3)   /* GPIOB 时钟                 */
#define RCC_APB2ENR_IOPCEN  (1UL << 4)   /* GPIOC 时钟                 */
#define RCC_APB2ENR_USART1EN (1UL << 14) /* USART1 时钟                */

/* RCC_APB1ENR 位定义 */
#define RCC_APB1ENR_TIM2EN  (1UL << 0)   /* TIM2 时钟                  */
#define RCC_APB1ENR_TIM3EN  (1UL << 1)   /* TIM3 时钟                  */
#define RCC_APB1ENR_USART2EN (1UL << 17) /* USART2 时钟                */

/* ============================ FLASH ============================ */

typedef struct {
    uint32_t ACR;         /* 0x00 访问控制寄存器（等待周期） */
} FLASH_TypeDef;

#define FLASH               ((FLASH_TypeDef *)FLASH_BASE)
#define FLASH_ACR_LATENCY_2 (2UL << 0)   /* 2 个等待周期（72MHz 需要） */

/* ============================ GPIO ============================ */

typedef struct {
    uint32_t CRL;         /* 0x00 端口配置低寄存器（引脚 0-7） */
    uint32_t CRH;         /* 0x04 端口配置高寄存器（引脚 8-15）*/
    uint32_t IDR;         /* 0x08 输入数据寄存器              */
    uint32_t ODR;         /* 0x0C 输出数据寄存器              */
    uint32_t BSRR;        /* 0x10 置位/复位寄存器             */
    uint32_t BRR;         /* 0x14 复位寄存器                  */
    uint32_t LCKR;        /* 0x18 配置锁定寄存器              */
} GPIO_TypeDef;

#define GPIOA               ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *)GPIOC_BASE)

/* GPIO 模式（用于 CRL/CRH 的 MODE 字段，2 位） */
#define GPIO_MODE_INPUT     (0UL)         /* 输入（复位态）       */
#define GPIO_MODE_OUT_10M   (1UL)         /* 输出 10MHz           */
#define GPIO_MODE_OUT_2M    (2UL)         /* 输出 2MHz            */
#define GPIO_MODE_OUT_50M   (3UL)         /* 输出 50MHz           */

/* GPIO 配置（CNF 字段，2 位） */
#define GPIO_CNF_IN_ANALOG  (0UL << 2)    /* 模拟输入             */
#define GPIO_CNF_IN_FLOAT   (1UL << 2)    /* 浮空输入             */
#define GPIO_CNF_IN_PUPD    (2UL << 2)    /* 上拉/下拉输入        */
#define GPIO_CNF_OUT_PP     (0UL << 2)    /* 通用推挽输出         */
#define GPIO_CNF_OUT_OD     (1UL << 2)    /* 通用开漏输出         */
#define GPIO_CNF_AF_PP      (2UL << 2)    /* 复用功能推挽输出     */
#define GPIO_CNF_AF_OD      (3UL << 2)    /* 复用功能开漏输出     */

/* 每个引脚在 CRL/CRH 中占 4 位，右移位数 = pin * 4 */
#define GPIO_PIN_SHIFT(pin) ((pin) * 4U)
/* 引脚 8~15 位于 CRH，其中右移位数 = (pin-8)*4 */
#define GPIO_CRH_SHIFT(pin) (((pin) - 8) * 4U)

/* ============================ USART ============================ */

typedef struct {
    uint32_t SR;          /* 0x00 状态寄存器    */
    uint32_t DR;          /* 0x04 数据寄存器    */
    uint32_t BRR;         /* 0x08 波特率寄存器  */
    uint32_t CR1;         /* 0x0C 控制寄存器 1  */
    uint32_t CR2;         /* 0x10 控制寄存器 2  */
    uint32_t CR3;         /* 0x14 控制寄存器 3  */
    uint32_t GTPR;        /* 0x18 保护时间寄存器 */
} USART_TypeDef;

#define USART1              ((USART_TypeDef *)USART1_BASE)
#define USART2              ((USART_TypeDef *)USART2_BASE)

/* USART_SR 位定义 */
#define USART_SR_TXE        (1UL << 7)   /* 发送数据寄存器空 */
#define USART_SR_TC         (1UL << 6)   /* 发送完成         */
#define USART_SR_RXNE       (1UL << 5)   /* 读数据寄存器非空 */

/* USART_CR1 位定义 */
#define USART_CR1_RXNEIE    (1UL << 5)   /* RXNE 中断使能    */
#define USART_CR1_TE        (1UL << 3)   /* 发送使能         */
#define USART_CR1_RE        (1UL << 2)   /* 接收使能         */
#define USART_CR1_UE        (1UL << 13)  /* USART 使能       */

/* ============================ TIM（通用定时器） ============================ */

typedef struct {
    uint32_t CR1;         /* 0x00 控制寄存器 1     */
    uint32_t CR2;         /* 0x04 控制寄存器 2     */
    uint32_t SMCR;        /* 0x08 从模式控制寄存器 */
    uint32_t DIER;        /* 0x0C DMA/中断使能     */
    uint32_t SR;          /* 0x10 状态寄存器       */
    uint32_t EGR;         /* 0x14 事件生成寄存器   */
    uint32_t CCMR1;       /* 0x18 捕获比较模式 1   */
    uint32_t CCMR2;       /* 0x1C 捕获比较模式 2   */
    uint32_t CCER;        /* 0x20 捕获比较使能     */
    uint32_t CNT;         /* 0x24 计数器           */
    uint32_t PSC;         /* 0x28 预分频器         */
    uint32_t ARR;         /* 0x2C 自动重装载寄存器 */
    uint32_t RCR;         /* 0x30 重复计数寄存器   */
    uint32_t CCR1;        /* 0x34 捕获/比较 1      */
    uint32_t CCR2;        /* 0x38 捕获/比较 2      */
    uint32_t CCR3;        /* 0x3C 捕获/比较 3      */
    uint32_t CCR4;        /* 0x40 捕获/比较 4      */
    uint32_t BDTR;        /* 0x44 刹车/死区寄存器  */
    uint32_t DCR;         /* 0x48 DMA 控制寄存器   */
    uint32_t DMAR;        /* 0x4C DMA 地址寄存器   */
} TIM_TypeDef;

#define TIM2                ((TIM_TypeDef *)TIM2_BASE)
#define TIM3                ((TIM_TypeDef *)TIM3_BASE)

/* TIM_CR1 位定义 */
#define TIM_CR1_CEN         (1UL << 0)   /* 计数器使能 */
#define TIM_CR1_ARPE        (1UL << 7)   /* 自动重装载预装载使能 */

/* TIM_CCMR1 位定义（输出比较通道 1/2） */
#define TIM_CCMR1_OC1M_PWM1 (6UL << 4)   /* OC1 PWM 模式 1 */
#define TIM_CCMR1_OC1PE     (1UL << 3)   /* OC1 预装载使能 */
#define TIM_CCMR1_OC2M_PWM1 (6UL << 12)  /* OC2 PWM 模式 1 */
#define TIM_CCMR1_OC2PE     (1UL << 11)  /* OC2 预装载使能 */

/* TIM_CCER 位定义 */
#define TIM_CCER_CC1E       (1UL << 0)   /* 捕获/比较 1 输出使能 */
#define TIM_CCER_CC2E       (1UL << 4)   /* 捕获/比较 2 输出使能 */

/* TIM_EGR 位定义 */
#define TIM_EGR_UG          (1UL << 0)   /* 更新事件生成 */

/* ============================ SysTick ============================ */

typedef struct {
    uint32_t CTRL;        /* 控制及状态寄存器 */
    uint32_t LOAD;        /* 重装载值寄存器   */
    uint32_t VAL;         /* 当前值寄存器     */
    uint32_t CALIB;       /* 校准值寄存器     */
} SysTick_TypeDef;

#define SysTick             ((SysTick_TypeDef *)0xE000E010UL)

#define SysTick_CTRL_ENABLE  (1UL << 0)
#define SysTick_CTRL_CLKSOURCE (1UL << 2)  /* 1 = 内核时钟，0 = 内核时钟/8 */
#define SysTick_CTRL_COUNTFLAG (1UL << 16)

#endif /* __STM32F10X_H */
