//
// Created by 52646 on 2026/11/20.
//

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

void    servo_init(void);
uint8_t servo_ok(void);
void    servo_set_norm(uint8_t ch, int16_t pos);
void    servo_set_target_us(uint8_t ch, uint16_t us);   /* 新增 */
int16_t servo_get_us(uint8_t ch);                        /* 新增 */
void    servo_task_10ms(void);

#endif /* SERVO_H */
