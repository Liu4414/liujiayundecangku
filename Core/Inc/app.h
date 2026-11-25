//
// Created by 52646 on 2026/11/20.
//

#ifndef APP_H
#define APP_H

#include <stdint.h>
#include "proto.h"

typedef enum {
    APP_FAILSAFE = 0,   /* 没收到过帧 / 超时 */
    APP_RUN,            /* 正常行驶 */
    APP_ESTOP,          /* 急停 */
    APP_BRAKE           /* L2 刹车 */
} app_state_t;

void        app_init(void);
void        app_on_frame(const rc_frame_t *f, uint32_t now);
void        app_task_10ms(uint32_t now);
void        app_set_manual(uint8_t on);
app_state_t app_state(void);

#endif /* APP_H */
