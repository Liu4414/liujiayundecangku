#include "config.h"
#include "arm.h"
#include "servo.h"

/* 5 路舵机角色（按你的机械结构改这张表）
 *   ARM_LIFT0 机械臂1 / ARM_LIFT1 机械臂2 / ARM_GRIP 夹爪
 *   ARM_TILT  收纳盒倾倒 / ARM_BOX 收纳盒升降
 * mirror=1 表示对称安装，输出取反
 */
typedef struct {
    uint8_t  ch;
    uint16_t min_us;
    uint16_t max_us;
    uint16_t home_us;
    uint8_t  mirror;
    int16_t  pos;
} arm_joint_t;

#define ARM_LIFT0   0
#define ARM_LIFT1   1
#define ARM_GRIP    2
#define ARM_TILT    3
#define ARM_BOX     4
#define ARM_JOINTS  5

static arm_joint_t J[ARM_JOINTS] = {
    /* ch  min   max   home  mirror pos */
    {  0, 1000, 2000, 1500, 0, 1500 },   /* 机械臂 1 */
    {  1, 1000, 2000, 1500, 0, 1500 },   /* 机械臂 2：对称安装改成 1 */
    {  2, 1000, 2000, 1500, 0, 1500 },   /* 夹爪 */
    {  3, 1000, 2000, 1500, 0, 1500 },   /* 收纳盒倾倒 */
    {  4, 1000, 2000, 1500, 0, 1500 }    /* 收纳盒升降 */
};

static void clamp_pos(arm_joint_t *j)
{
    if (j->pos < (int16_t)j->min_us) j->pos = (int16_t)j->min_us;
    if (j->pos > (int16_t)j->max_us) j->pos = (int16_t)j->max_us;
}

static void move_joint(uint8_t idx, int32_t input, int32_t rate)
{
    int32_t d;

    if (input == 0) return;

    d = rate * input / 10000;          /* 10ms 周期 */
    if (d == 0) d = (input > 0) ? 1 : -1;

    J[idx].pos = (int16_t)((int32_t)J[idx].pos + d);
    clamp_pos(&J[idx]);
}

static void apply(void)
{
    uint8_t i;

    for (i = 0; i < ARM_JOINTS; i++) {
        uint16_t us = (uint16_t)J[i].pos;
        if (J[i].mirror) {
            us = (uint16_t)(J[i].min_us + J[i].max_us - us);
        }
        servo_set_target_us(J[i].ch, us);
    }
}

void arm_init(void)
{
    uint8_t i;
    for (i = 0; i < ARM_JOINTS; i++) J[i].pos = (int16_t)J[i].home_us;
    apply();
}

void arm_home(void)
{
    uint8_t i;
    for (i = 0; i < ARM_JOINTS; i++) J[i].pos = (int16_t)J[i].home_us;
    apply();
}

void arm_update(const rc_frame_t *f)
{
    uint16_t btn = f->btn;
    int32_t  in;

    /* 机械臂升降：右摇杆 Y */
    in = f->ry;
    if ((in > -ARM_STICK_DZ) && (in < ARM_STICK_DZ)) in = 0;
    move_joint(ARM_LIFT0, in, ARM_STICK_RATE);
    move_joint(ARM_LIFT1, in, ARM_STICK_RATE);

    /* 夹爪：L1 张开 / R1 闭合 */
    in = 0;
    if (btn & BTN_L1) in += 100;
    if (btn & BTN_R1) in -= 100;
    move_joint(ARM_GRIP, in, ARM_BTN_RATE);

    /* 收纳盒倾倒：□ / △ */
    in = 0;
    if (btn & BTN_SQUARE)   in += 100;
    if (btn & BTN_TRIANGLE) in -= 100;
    move_joint(ARM_TILT, in, ARM_BTN_RATE);

    /* 收纳盒升降：十字 ↑ / ↓ */
    in = 0;
    if (btn & BTN_DPAD_UP)   in += 100;
    if (btn & BTN_DPAD_DOWN) in -= 100;
    move_joint(ARM_BOX, in, ARM_BTN_RATE);

    /* ○：全部回中 */
    if (btn & BTN_CIRCLE) arm_home();

    apply();
}
