//
// Created by 52646 on 2026/11/19.
//

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

typedef enum {
    MOTOR_L1 = 0,   /* 左前 */
    MOTOR_R1,       /* 右前 */
    MOTOR_L2,       /* 左中 */
    MOTOR_R2,       /* 右中 */
    MOTOR_L3,       /* 左后 */
    MOTOR_R3,       /* 右后 */
    MOTOR_NUM
} motor_id_t;

void    motor_init(void);
void    motor_enable(uint8_t on);
void    motor_set(motor_id_t id, int16_t permille);
void    motor_stop(motor_id_t id);
void    motor_brake(motor_id_t id);
void    motor_stop_all(void);
int16_t motor_get(motor_id_t id);

#endif /* MOTOR_H */
