//
// Created by 52646 on 2026/11/20.
//

#ifndef CHASSIS_H
#define CHASSIS_H

#include <stdint.h>

/* vx: 前进(+)/后退(-)   vy: 左移(+)/右移(-)   omega: 左转(+)/右转(-)
 * 三个量范围都是 -1000 ~ +1000
 */
void chassis_init(void);
void chassis_set(int16_t vx, int16_t vy, int16_t omega);
void chassis_stop(void);
void chassis_task_10ms(void);

#endif /* CHASSIS_H */
