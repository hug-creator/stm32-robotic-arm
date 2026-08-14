/**
 * ============================================================================
 * @file    main.c
 * @brief   四自由度机械臂控制主程序（STM32F103C8T6 + 4× 串口舵机）
 *
 * 硬件资源：
 *   - 舵机：4 个 Fashion Star 串口舵机，接 USART2（PA2=TX, PA3=RX）
 *           ID 分配：1=底座旋转  2=大臂  3=小臂  4=夹爪
 *   - 按键：PB12=复位  PB13=夹取(搬运动作)  PB14=欢迎(挥手)  PB15=停止
 *   - 指示灯：PB10、PB11
 *
 * 工作流程：上电后进入主循环，扫描按键，触发对应动作序列。
 * 动作序列采用"关键帧"方式定义：每个关键帧描述"某舵机转到某角度、用时多久"，
 * 由 play_motion() 逐帧下发，结构清晰、易于修改和扩展。
 * ============================================================================
 */

#include "stm32f10x.h"
#include "delay.h"
#include "uart.h"
#include "servo.h"

/* ============================ GPIO 引脚定义 ============================ */

#define LED1_PIN        10      /* PB10 */
#define LED2_PIN        11      /* PB11 */
#define KEY_HOME_PIN    12      /* PB12：复位   */
#define KEY_PICK_PIN    13      /* PB13：夹取   */
#define KEY_WAVE_PIN    14      /* PB14：欢迎   */
#define KEY_STOP_PIN    15      /* PB15：停止   */

#define KEY_HOME_BIT    (1UL << KEY_HOME_PIN)
#define KEY_PICK_BIT    (1UL << KEY_PICK_PIN)
#define KEY_WAVE_BIT    (1UL << KEY_WAVE_PIN)
#define KEY_STOP_BIT    (1UL << KEY_STOP_PIN)

/* 舵机 ID 定义 */
#define SERVO_BASE      1       /* 底座旋转关节 */
#define SERVO_SHOULDER  2       /* 大臂关节     */
#define SERVO_ELBOW     3       /* 小臂关节     */
#define SERVO_GRIPPER   4       /* 夹爪         */

/* 舵机运动参数 */
#define MOVE_INTERVAL   600     /* 单关节默认运动耗时（ms） */
#define MOVE_POWER      0       /* 功率（0 = 舵机默认） */

/* ============================ 动作序列 ============================ */

/* 动作关键帧：某舵机运动到某角度，用时 interval 毫秒 */
typedef struct {
    uint8_t  servo_id;
    float    angle;       /* 目标角度（度） */
    uint16_t interval;    /* 运动耗时（ms） */
} MotionStep;

/**
 * 复位动作：所有关节依次回到 0° 收纳位
 * 顺序：夹爪先张开 -> 小臂 -> 大臂 -> 底座，避免运动干涉
 */
static const MotionStep motion_home[] = {
    { SERVO_GRIPPER,  30.0f, MOVE_INTERVAL },  /* 先松开夹爪 */
    { SERVO_ELBOW,     0.0f, MOVE_INTERVAL },
    { SERVO_SHOULDER,  0.0f, MOVE_INTERVAL },
    { SERVO_BASE,      0.0f, MOVE_INTERVAL },
    { SERVO_GRIPPER,   0.0f, MOVE_INTERVAL },  /* 夹爪合拢收纳 */
};

/**
 * 夹取动作：从左/右一侧夹取物体，搬运到另一侧放下（搬运动作）
 */
static const MotionStep motion_pick[] = {
    /* —— 1. 张开夹爪，转向左侧 —— */
    { SERVO_GRIPPER,  30.0f, MOVE_INTERVAL },
    { SERVO_BASE,    -90.0f, MOVE_INTERVAL },
    /* —— 2. 下降接近物体 —— */
    { SERVO_SHOULDER, -65.0f, MOVE_INTERVAL },
    { SERVO_ELBOW,    -20.0f, MOVE_INTERVAL },
    /* —— 3. 合拢夹爪，抓取 —— */
    { SERVO_GRIPPER,  10.0f, MOVE_INTERVAL },
    /* —— 4. 抬起手臂 —— */
    { SERVO_SHOULDER,   0.0f, MOVE_INTERVAL },
    { SERVO_ELBOW,      0.0f, MOVE_INTERVAL },
    /* —— 5. 转向右侧 —— */
    { SERVO_BASE,      90.0f, MOVE_INTERVAL },
    /* —— 6. 下降放置 —— */
    { SERVO_SHOULDER, -65.0f, MOVE_INTERVAL },
    { SERVO_ELBOW,    -20.0f, MOVE_INTERVAL },
    /* —— 7. 松开夹爪，放下物体 —— */
    { SERVO_GRIPPER,  30.0f, MOVE_INTERVAL },
    /* —— 8. 回正 —— */
    { SERVO_SHOULDER,   0.0f, MOVE_INTERVAL },
    { SERVO_ELBOW,      0.0f, MOVE_INTERVAL },
    { SERVO_BASE,       0.0f, MOVE_INTERVAL },
    { SERVO_GRIPPER,    0.0f, MOVE_INTERVAL },
};

/**
 * 欢迎动作：小臂摆动 + 底座小幅摆动，模拟挥手致意
 */
static const MotionStep motion_wave[] = {
    { SERVO_ELBOW,  -45.0f, MOVE_INTERVAL },
    { SERVO_BASE,    50.0f, MOVE_INTERVAL },
    { SERVO_ELBOW,   20.0f, MOVE_INTERVAL },
    { SERVO_BASE,   -50.0f, MOVE_INTERVAL },
    { SERVO_ELBOW,   20.0f, MOVE_INTERVAL },
    { SERVO_BASE,    50.0f, MOVE_INTERVAL },
    { SERVO_ELBOW,  -45.0f, MOVE_INTERVAL },
    { SERVO_BASE,     0.0f, MOVE_INTERVAL },
    { SERVO_ELBOW,    0.0f, MOVE_INTERVAL },
};

/* ============================ 系统时钟 ============================ */

/**
 * @brief 配置系统时钟为 72MHz（HSE 8MHz × PLL9）
 *        同时设置 AHB/APB2 = 72MHz，APB1 = 36MHz
 */
static void system_clock_config(void)
{
    uint32_t tmp;

    /* 1. 使能外部高速时钟 HSE，等待就绪 */
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) {
        ;
    }

    /* 2. 配置 FLASH 等待周期为 2（72MHz 下必需） */
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    /* 3. 配置 PLL：HSE 源、9 倍频；APB1 二分频（36MHz），APB2 不分频（72MHz） */
    tmp = RCC->CFGR;
    tmp &= ~((0xFUL << 18) | (0x7UL << 8) | (0x7UL << 11) | (0x1UL << 16));
    tmp |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9 | (4UL << 8);
    RCC->CFGR = tmp;

    /* 4. 使能 PLL，等待就绪 */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {
        ;
    }

    /* 5. 将 PLL 切换为系统时钟 */
    tmp = RCC->CFGR;
    tmp &= ~0x3UL;
    tmp |= RCC_CFGR_SW_PLL;
    RCC->CFGR = tmp;

    /* 6. 等待切换完成 */
    while ((RCC->CFGR & 0xCUL) != RCC_CFGR_SWS_PLL) {
        ;
    }
}

/* ============================ GPIO ============================ */

/**
 * @brief GPIO 初始化：LED 推挽输出，按键上拉输入
 */
static void gpio_config(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /* LED（PB10、PB11）：通用推挽输出，50MHz */
    GPIOB->CRH &= ~(0xFFUL << GPIO_CRH_SHIFT(10));
    GPIOB->CRH |= (GPIO_CNF_OUT_PP | GPIO_MODE_OUT_50M) << GPIO_CRH_SHIFT(LED1_PIN);
    GPIOB->CRH |= (GPIO_CNF_OUT_PP | GPIO_MODE_OUT_50M) << GPIO_CRH_SHIFT(LED2_PIN);

    /* 按键（PB12~PB15）：上拉输入（按下为低电平） */
    GPIOB->CRH &= ~(0xFFFFUL << GPIO_CRH_SHIFT(12));
    GPIOB->CRH |= (GPIO_CNF_IN_PUPD | GPIO_MODE_INPUT) << GPIO_CRH_SHIFT(KEY_HOME_PIN);
    GPIOB->CRH |= (GPIO_CNF_IN_PUPD | GPIO_MODE_INPUT) << GPIO_CRH_SHIFT(KEY_PICK_PIN);
    GPIOB->CRH |= (GPIO_CNF_IN_PUPD | GPIO_MODE_INPUT) << GPIO_CRH_SHIFT(KEY_WAVE_PIN);
    GPIOB->CRH |= (GPIO_CNF_IN_PUPD | GPIO_MODE_INPUT) << GPIO_CRH_SHIFT(KEY_STOP_PIN);
    GPIOB->ODR |= (KEY_HOME_BIT | KEY_PICK_BIT | KEY_WAVE_BIT | KEY_STOP_BIT); /* 上拉 */
}

/* ============================ 动作执行 ============================ */

/**
 * @brief 等待舵机旋转到位，同时检测停止键（闭环控制 + 可随时急停）
 * @param servo_id 舵机 ID
 * @param angle    目标角度（度）
 * @param interval 预期运动耗时（ms），也作为等待超时上限
 * @return 1 = 被停止键中断，0 = 到位或超时
 */
static uint8_t motion_wait(uint8_t servo_id, float angle, uint16_t interval)
{
    float cur;
    uint32_t elapsed = 0;

    while (elapsed < interval) {
        /* 1. 检测停止键（低电平有效），消抖后确认 */
        if (!(GPIOB->IDR & KEY_STOP_BIT)) {
            delay_ms(20);
            if (!(GPIOB->IDR & KEY_STOP_BIT)) {
                return 1;                   /* 停止键按下，中断动作 */
            }
        }

        /* 2. 回读角度，判断是否到位 */
        if (servo_read_angle(servo_id, &cur)) {
            float err = angle - cur;
            if (err < 0) {
                err = -err;
            }
            if (err <= FSUS_ANGLE_DEADAREA) {
                return 0;                   /* 已到位，提前进入下一帧 */
            }
        }

        delay_ms(20);
        elapsed += 20;
    }
    return 0;                               /* 超时，进入下一关键帧 */
}

/**
 * @brief 逐帧执行一套动作序列
 * @param motion 关键帧数组
 * @param count  关键帧数量
 * @note  每个关键帧下发后，闭环等待舵机到位；期间可被停止键急停
 */
static void play_motion(const MotionStep *motion, uint16_t count)
{
    uint16_t i;

    for (i = 0; i < count; i++) {
        servo_set_angle(motion[i].servo_id, motion[i].angle,
                        motion[i].interval, MOVE_POWER);
        if (motion_wait(motion[i].servo_id, motion[i].angle, motion[i].interval)) {
            return;                         /* 被停止键中断，结束动作 */
        }
    }
}

/* ============================ 按键扫描 ============================ */

/**
 * @brief 扫描按键，消抖并触发对应动作
 * @note  停止键不在此处处理：它由 play_motion() 内部的 motion_wait()
 *        在动作执行期间实时检测，用于急停当前动作
 */
static void check_keys(void)
{
    /* 复位键 */
    if (!(GPIOB->IDR & KEY_HOME_BIT)) {
        delay_ms(20);
        if (!(GPIOB->IDR & KEY_HOME_BIT)) {
            GPIOB->ODR &= ~(1UL << LED1_PIN);           /* 点亮 LED 指示 */
            play_motion(motion_home, sizeof(motion_home) / sizeof(MotionStep));
            GPIOB->ODR |= (1UL << LED1_PIN);
            while (!(GPIOB->IDR & KEY_HOME_BIT)) {
                ;
            }
        }
    }

    /* 夹取键 */
    if (!(GPIOB->IDR & KEY_PICK_BIT)) {
        delay_ms(20);
        if (!(GPIOB->IDR & KEY_PICK_BIT)) {
            GPIOB->ODR &= ~(1UL << LED2_PIN);
            play_motion(motion_pick, sizeof(motion_pick) / sizeof(MotionStep));
            GPIOB->ODR |= (1UL << LED2_PIN);
            while (!(GPIOB->IDR & KEY_PICK_BIT)) {
                ;
            }
        }
    }

    /* 欢迎键 */
    if (!(GPIOB->IDR & KEY_WAVE_BIT)) {
        delay_ms(20);
        if (!(GPIOB->IDR & KEY_WAVE_BIT)) {
            GPIOB->ODR &= ~(1UL << LED1_PIN);
            play_motion(motion_wave, sizeof(motion_wave) / sizeof(MotionStep));
            GPIOB->ODR |= (1UL << LED1_PIN);
            while (!(GPIOB->IDR & KEY_WAVE_BIT)) {
                ;
            }
        }
    }
}

/* ============================ 主函数 ============================ */

int main(void)
{
    system_clock_config();          /* 系统时钟 72MHz */
    delay_init();                   /* 初始化 SysTick 延时 */
    uart_init();                    /* 初始化串口（舵机 + 调试） */
    gpio_config();                  /* 初始化 LED 与按键 */

    /* 上电自检：亮一下 LED，打印启动信息 */
    GPIOB->ODR &= ~((1UL << LED1_PIN) | (1UL << LED2_PIN));
    delay_ms(200);
    GPIOB->ODR |= (1UL << LED1_PIN) | (1UL << LED2_PIN);

    uart_send_string(DEBUG_UART, "Robotic Arm Ready\r\n");

    while (1) {
        check_keys();
    }
}
