#include "config.h"
#include "arm.h"
#include "servo.h"

/* ============================================================
 * 5 路舵机角色（逻辑通道 -> PCA9685 物理通道见 config.h 的 SERVO_CH_LIST）
 *
 *   逻辑 1 -> CH11  车斗仰起 / 倾倒      十字 上 / 下
 *   逻辑 2 -> CH12  夹爪                 L1 张开 / L2 闭合
 *   逻辑 3 -> CH13  机械臂升降           右摇杆 上 / 下
 *   逻辑 4 -> CH14  机械臂前伸 / 后缩    R1 前伸 / R2 后缩
 *   逻辑 5 -> CH15  云台左转 / 右转      十字 左 / 右
 *
 * mirror = 1 表示对称安装，输出取反
 * ============================================================ */
typedef struct {
    uint8_t  ch;          /* 逻辑通道 0~4，对应 CH11~CH15 */
    uint16_t min_us;
    uint16_t max_us;
    uint16_t home_us;
    uint8_t  mirror;
    int16_t  pos;
} arm_joint_t;

#define ARM_DUMP    0     /* CH11 车斗 */
#define ARM_GRIP    1     /* CH12 夹爪 */
#define ARM_LIFT    2     /* CH13 机械臂升降 */
#define ARM_EXT     3     /* CH14 机械臂前后 */
#define ARM_PAN     4     /* CH15 云台 */
#define ARM_JOINTS  5

static arm_joint_t J[ARM_JOINTS] = {
    /* ch  min   max   home  mirror pos */
    {  0, 1000, 1900, 1500, 0, 1500 },   /* CH11 车斗 */
    {  1, 1000, 1900, 1500, 0, 1500 },   /* CH12 夹爪 */
    {  2, 1000, 1900, 1275, 0, 1275 },   /* CH13 机械臂升降：归中下调 45 度 */
    {  3, 1000, 1900, 1500, 0, 1500 },   /* CH14 机械臂前后 */
    {  4, 1000, 2000, 1500, 0, 1500 }    /* CH15 云台 360 度连续舵机 */
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

/* ============================================================
 * 每 10ms 调用一次
 * ============================================================ */
void arm_update(const rc_frame_t *f)
{
    uint16_t btn = f->btn;
    int32_t  in;

    /* ---- 机械臂升降：右摇杆 上 / 下 ---- */
    in = f->ry;
    if ((in > -ARM_STICK_DZ) && (in < ARM_STICK_DZ)) in = 0;
    move_joint(ARM_LIFT, in, ARM_STICK_RATE);

    /* ---- 夹爪：L1 张开(+) / L2 闭合(-) ---- */
    in = 0;
    if (btn & BTN_L1)  in += 100;
    if (f->l2 > 128u)  in -= 100;
    move_joint(ARM_GRIP, in, ARM_BTN_RATE);

    /* ---- 机械臂前伸后缩：R1 后缩(-) / R2 前伸(+) ---- */
    in = 0;
    if (btn & BTN_R1)  in -= 100;
    if (f->r2 > 128u)  in += 100;
    move_joint(ARM_EXT, in, ARM_BTN_RATE);

    /* ---- 云台：360 度连续舵机，只控转速，松开立刻停 ----
     * 1500 = 停；>1500 正转，<1500 反转
     * 方向反了就把下面两行的 +/- 对调
     */
    {
        uint16_t us = ARM_PAN_STOP_US;

        if (btn & BTN_DPAD_RIGHT) us = (uint16_t)(ARM_PAN_STOP_US + ARM_PAN_SPEED_US);
        if (btn & BTN_DPAD_LEFT)  us = (uint16_t)(ARM_PAN_STOP_US - ARM_PAN_SPEED_US);

        J[ARM_PAN].pos = (int16_t)us;
        servo_set_target_us(ARM_PAN, us);
    }

    /* ---- 车斗：十字 上 = 仰起(+) / 下 = 倾倒(-) ---- */
    in = 0;
    if (btn & BTN_DPAD_UP)   in += 100;
    if (btn & BTN_DPAD_DOWN) in -= 100;
    move_joint(ARM_DUMP, in, ARM_BTN_RATE);

    /* ---- ○：全部回中 ---- */
    if (btn & BTN_CIRCLE) arm_home();

    apply();
}
