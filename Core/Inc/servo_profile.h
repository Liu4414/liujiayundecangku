//
// Created by 52646 on 2026/11/20.
//

#ifndef SERVO_PROFILE_H
#define SERVO_PROFILE_H

#include <stdint.h>

typedef struct {
    int32_t pos;      /* 当前位置，单位 us */
    int32_t vel;      /* 当前速度，单位 us/s */
    int32_t target;   /* 目标位置，单位 us */
    int32_t vmax;     /* 最大速度，单位 us/s */
    int32_t accel;    /* 加速度，单位 us/s^2 */
} servo_prof_t;

void    servo_prof_init(servo_prof_t *p, int32_t pos, int32_t vmax, int32_t accel);
void    servo_prof_set_target(servo_prof_t *p, int32_t target);
void    servo_prof_update(servo_prof_t *p, int32_t dt_ms);
uint8_t servo_prof_done(const servo_prof_t *p);

#endif /* SERVO_PROFILE_H */
