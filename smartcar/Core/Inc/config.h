//
// Created by 52646 on 2026/11/19.
//

#ifndef CONFIG_H
#define CONFIG_H

#include "main.h"
#include "tim.h"

/* ============================================================
 * 电机 1：左前 L1（TB6612 #1 的 A 通道）
 * ============================================================ */
#define M1_PWM_HTIM    (&htim3)
#define M1_PWM_CH      TIM_CHANNEL_1
#define M1_IN1_PORT    GPIOA
#define M1_IN1_PIN     GPIO_PIN_0
#define M1_IN2_PORT    GPIOA
#define M1_IN2_PIN     GPIO_PIN_1
#define M1_INVERT      0

/* ============================================================
 * 电机 2：右前 R1（TB6612 #1 的 B 通道）
 * ============================================================ */
#define M2_PWM_HTIM    (&htim3)
#define M2_PWM_CH      TIM_CHANNEL_2
#define M2_IN1_PORT    GPIOA
#define M2_IN1_PIN     GPIO_PIN_4
#define M2_IN2_PORT    GPIOA
#define M2_IN2_PIN     GPIO_PIN_5
#define M2_INVERT      0

/* ============================================================
 * 电机 3：左中 L2（TB6612 #2 的 A 通道）
 * ============================================================ */
#define M3_PWM_HTIM    (&htim3)
#define M3_PWM_CH      TIM_CHANNEL_3
#define M3_IN1_PORT    GPIOA
#define M3_IN1_PIN     GPIO_PIN_8
#define M3_IN2_PORT    GPIOA
#define M3_IN2_PIN     GPIO_PIN_11
#define M3_INVERT      0

/* ============================================================
 * 电机 4：右中 R2（TB6612 #2 的 B 通道）
 * ============================================================ */
#define M4_PWM_HTIM    (&htim3)
#define M4_PWM_CH      TIM_CHANNEL_4
#define M4_IN1_PORT    GPIOA
#define M4_IN1_PIN     GPIO_PIN_12
#define M4_IN2_PORT    GPIOB
#define M4_IN2_PIN     GPIO_PIN_8
#define M4_INVERT      0

/* ============================================================
 * 电机 5：左后 L3（TB6612 #3 的 A 通道）
 * ============================================================ */
#define M5_PWM_HTIM    (&htim4)
#define M5_PWM_CH      TIM_CHANNEL_1
#define M5_IN1_PORT    GPIOB
#define M5_IN1_PIN     GPIO_PIN_9
#define M5_IN2_PORT    GPIOB
#define M5_IN2_PIN     GPIO_PIN_12
#define M5_INVERT      0

/* ============================================================
 * 电机 6：右后 R3（TB6612 #3 的 B 通道）
 * ============================================================ */
#define M6_PWM_HTIM    (&htim4)
#define M6_PWM_CH      TIM_CHANNEL_2
#define M6_IN1_PORT    GPIOB
#define M6_IN1_PIN     GPIO_PIN_13
#define M6_IN2_PORT    GPIOB
#define M6_IN2_PIN     GPIO_PIN_15
#define M6_INVERT      0

/* ============================================================
 * 公共：三片 TB6612 的 STBY 并联到 PB14
 * ============================================================ */
#define MOTOR_STBY_PORT   GPIOB
#define MOTOR_STBY_PIN    GPIO_PIN_14

/* 最小占空比（千分比）。0 = 不用；低速不转时改成 30~60 */
#define MOTOR_MIN_DUTY    0

/* ============================================================
 * 底盘运动学参数
 * ============================================================ */
#define CHASSIS_ACCEL        60    /* 每 10ms 允许变化量（满量程 1000） */
#define CHASSIS_OMEGA_GAIN   600   /* 旋转权重，实车再调 */
#define CHASSIS_MAX          1000

/* 摇杆到车体坐标的符号（装反只改这里） */
#define APP_VX_SIGN   (+1)
#define APP_VY_SIGN   (-1)
#define APP_W_SIGN    (-1)

/* ============================================================
 * 舵机
 * ============================================================ */
#define SERVO_NUM         5
#define SERVO_MIN_US      1000
#define SERVO_MID_US      1500
#define SERVO_MAX_US      2000
#define SERVO_VMAX        800     /* us/s */
#define SERVO_ACCEL       4000    /* us/s^2 */

#define PCA9685_I2C_ADDR  0x40
#define PCA9685_PWM_HZ    50

/* ============================================================
 * 安全
 * ============================================================ */
#define APP_TIMEOUT_MS    200

#endif /* CONFIG_H */
