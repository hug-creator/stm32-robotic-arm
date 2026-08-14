/**
 * ============================================================================
 * @file    servo.h
 * @brief   Fashion Star（法山）串口舵机驱动声明
 *
 * 本驱动实现了 FSUS（Fashion Star UART Servo）通信协议，用于通过串口
 * 控制串口舵机的角度。协议帧格式：
 *
 *   [帧头 2B][指令ID 1B][数据长度 1B][数据 N B][校验和 1B]
 *   - 帧头：0x4C 0x12（小端）
 *   - 校验和：帧内所有字节（含帧头）求和，取低 8 位
 *
 * 本项目机械臂使用 4 个串口舵机（ID = 1~4），通过 USART2 通信。
 * ============================================================================
 */

#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f10x.h"

/* FSUS 协议常量 */
#define FSUS_HEADER_LOW     0x12    /* 帧头低字节 */
#define FSUS_HEADER_HIGH    0x4C    /* 帧头高字节 */

#define FSUS_CMD_ROTATE     8       /* 角度控制指令（设置舵机角度） */
#define FSUS_CMD_DAMPING    9       /* 阻尼模式指令 */
#define FSUS_CMD_READ_ANGLE 10      /* 读取舵机角度指令 */

/* 舵机角度范围（单位：度） */
#define SERVO_ANGLE_MAX     180.0f
#define SERVO_ANGLE_MIN     (-180.0f)

/* 角度到位死区（单位：度）：回读角度与目标角度误差小于该值视为到位 */
#define FSUS_ANGLE_DEADAREA 1.0f

/**
 * @brief 设置舵机角度
 * @param servo_id 舵机 ID（1~254）
 * @param angle    目标角度（度，范围 -180 ~ +180，精度 0.1°）
 * @param interval 到达目标角度所用时间（毫秒）
 * @param power    舵机执行功率（毫瓦，0 表示使用舵机默认值）
 * @note  本函数为“非阻塞”发送：仅下发指令，不等待舵机到位
 */
void servo_set_angle(uint8_t servo_id, float angle, uint16_t interval, uint16_t power);

/**
 * @brief 回读舵机当前角度（闭环控制的核心）
 * @param servo_id 舵机 ID
 * @param angle    输出：当前角度（度）
 * @return 1 = 成功，0 = 失败（超时 / 帧头错误 / ID 不匹配）
 */
uint8_t servo_read_angle(uint8_t servo_id, float *angle);

/**
 * @brief 让舵机进入阻尼模式（卸力，便于手动调整姿态）
 * @param servo_id 舵机 ID
 * @param power    阻尼功率（毫瓦）
 */
void servo_damping(uint8_t servo_id, uint16_t power);

#endif /* __SERVO_H */
