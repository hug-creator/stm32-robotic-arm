/**
 * ============================================================================
 * @file    servo.c
 * @brief   Fashion Star 串口舵机驱动实现（FSUS 协议）
 *
 * 帧格式：| 帧头 2B | 指令ID 1B | 数据长度 1B | 数据 N B | 校验和 1B |
 * 校验和 = 帧内所有字节（含帧头）相加，取低 8 位。
 *
 * 所有多字节字段均按小端序（Little Endian）编码。
 * ============================================================================
 */

#include "servo.h"
#include "uart.h"

/* 发送帧最大长度（帧头 2 + 指令 1 + 长度 1 + 数据 15 + 校验 1） */
#define FSUS_MAX_FRAME_LEN  20U

/**
 * @brief 构造并发送一帧 FSUS 指令
 * @param cmd     指令 ID
 * @param data    数据负载
 * @param len     数据负载长度（字节）
 */
static void servo_send_frame(uint8_t cmd, const uint8_t *data, uint8_t len)
{
    uint8_t frame[FSUS_MAX_FRAME_LEN];
    uint16_t checksum = 0;
    uint8_t i = 0;
    uint8_t idx = 0;

    /* 1. 帧头（小端：先发低字节） */
    frame[idx++] = FSUS_HEADER_LOW;
    frame[idx++] = FSUS_HEADER_HIGH;

    /* 2. 指令 ID */
    frame[idx++] = cmd;

    /* 3. 数据长度 */
    frame[idx++] = len;

    /* 4. 数据负载 */
    for (i = 0; i < len; i++) {
        frame[idx++] = data[i];
    }

    /* 5. 校验和 = 前面所有字节求和，取低 8 位 */
    for (i = 0; i < idx; i++) {
        checksum += frame[i];
    }
    frame[idx++] = (uint8_t)(checksum & 0xFF);

    /* 通过舵机串口发出 */
    uart_send_bytes(SERVO_UART, frame, idx);
}

/**
 * @brief 设置舵机角度
 *
 * 数据负载（7 字节）：
 *   [舵机ID 1B][角度 int16 0.1° 小端][周期 uint16 ms 小端][功率 uint16 mW 小端]
 */
void servo_set_angle(uint8_t servo_id, float angle, uint16_t interval, uint16_t power)
{
    uint8_t data[7];
    int16_t angle_q10;      /* 角度放大 10 倍，精度 0.1° */

    /* 角度限幅 */
    if (angle > SERVO_ANGLE_MAX) {
        angle = SERVO_ANGLE_MAX;
    } else if (angle < SERVO_ANGLE_MIN) {
        angle = SERVO_ANGLE_MIN;
    }

    angle_q10 = (int16_t)(angle * 10.0f);

    data[0] = servo_id;
    data[1] = (uint8_t)(angle_q10 & 0xFF);          /* 角度低字节 */
    data[2] = (uint8_t)((angle_q10 >> 8) & 0xFF);   /* 角度高字节 */
    data[3] = (uint8_t)(interval & 0xFF);           /* 周期低字节 */
    data[4] = (uint8_t)((interval >> 8) & 0xFF);    /* 周期高字节 */
    data[5] = (uint8_t)(power & 0xFF);              /* 功率低字节 */
    data[6] = (uint8_t)((power >> 8) & 0xFF);       /* 功率高字节 */

    servo_send_frame(FSUS_CMD_ROTATE, data, 7);
}

/**
 * @brief 舵机阻尼模式（卸力）
 *
 * 数据负载（3 字节）：[舵机ID 1B][功率 uint16 mW 小端]
 */
void servo_damping(uint8_t servo_id, uint16_t power)
{
    uint8_t data[3];

    data[0] = servo_id;
    data[1] = (uint8_t)(power & 0xFF);
    data[2] = (uint8_t)((power >> 8) & 0xFF);

    servo_send_frame(FSUS_CMD_SPIN, data, 3);
}
