#include "config.h"
#include "chassis.h"
#include "motor.h"

/* ============================================================
 * 混控矩阵：每个轮子对 vx / vy / omega 的贡献系数
 *
 *   ax：前进系数（都是 1）
 *   ay：横移系数（麦轮才有力，全向轮为 0）
 *   aw：自转系数（左右两侧相反）
 *
 *   轮序：L1 左前 / R1 右前 / L2 左中 / R2 右中 / L3 左后 / R3 右后
 *
 *   如果实车某个方向反了，优先改 motor 的 INVERT；
 *   如果只有横移方向反了（麦轮辊子装反），把该行 ay 取反。
 * ============================================================ */
typedef struct {
    int8_t ax;
    int8_t ay;
    int8_t aw;
} mix_t;

static const mix_t MIX[MOTOR_NUM] = {
    {  1, -1, -1 },   /* L1 左前 麦轮 */
    {  1,  1,  1 },   /* R1 右前 麦轮 */
    {  1,  0, -1 },   /* L2 左中 全向 */
    {  1,  0,  1 },   /* R2 右中 全向 */
    {  1,  1, -1 },   /* L3 左后 麦轮 */
    {  1, -1,  1 }    /* R3 右后 麦轮 */
};

static int16_t s_vx, s_vy, s_w;   /* 当前值（经过斜坡） */
static int16_t t_vx, t_vy, t_w;   /* 目标值 */

/* ------------------------------------------------------------
 * 限幅到 ±1000
 * ------------------------------------------------------------ */
static int16_t clamp1000(int32_t v)
{
    if (v >  1000) return  1000;
    if (v < -1000) return -1000;
    return (int16_t)v;
}

/* ------------------------------------------------------------
 * 斜坡：每个 10ms 最多朝目标走 CHASSIS_ACCEL
 * ------------------------------------------------------------ */
static int16_t ramp_step(int16_t cur, int16_t tgt)
{
    int32_t d = (int32_t)tgt - (int32_t)cur;

    if (d >  CHASSIS_ACCEL) d =  CHASSIS_ACCEL;
    if (d < -CHASSIS_ACCEL) d = -CHASSIS_ACCEL;

    return (int16_t)((int32_t)cur + d);
}

/* ------------------------------------------------------------
 * 初始化
 * ------------------------------------------------------------ */
void chassis_init(void)
{
    s_vx = s_vy = s_w = 0;
    t_vx = t_vy = t_w = 0;
}

/* ------------------------------------------------------------
 * 设置目标速度（-1000 ~ +1000）
 * ------------------------------------------------------------ */
void chassis_set(int16_t vx, int16_t vy, int16_t omega)
{
    t_vx = clamp1000(vx);
    t_vy = clamp1000(vy);
    t_w  = clamp1000(omega);
}

/* ------------------------------------------------------------
 * 立即停车（不走斜坡）
 * ------------------------------------------------------------ */
void chassis_stop(void)
{
    s_vx = s_vy = s_w = 0;
    t_vx = t_vy = t_w = 0;
    motor_stop_all();
}

/* ------------------------------------------------------------
 * 10ms 周期任务：斜坡 → 混控 → 归一化 → 下发
 * ------------------------------------------------------------ */
void chassis_task_10ms(void)
{
    int32_t u[MOTOR_NUM];
    int32_t max = 0;
    int32_t rot;
    uint8_t i;

    s_vx = ramp_step(s_vx, t_vx);
    s_vy = ramp_step(s_vy, t_vy);
    s_w  = ramp_step(s_w,  t_w);

    rot = (int32_t)s_w * CHASSIS_OMEGA_GAIN / 1000;

    for (i = 0; i < MOTOR_NUM; i++) {
        u[i] = (int32_t)MIX[i].ax * s_vx
             + (int32_t)MIX[i].ay * s_vy
             + (int32_t)MIX[i].aw * rot;

        if (u[i] >  max) max =  u[i];
        if (-u[i] > max) max = -u[i];
    }

    /* 任何一路超过 1000，就整体等比缩小，保持方向不变 */
    if (max > CHASSIS_MAX) {
        for (i = 0; i < MOTOR_NUM; i++) {
            u[i] = u[i] * CHASSIS_MAX / max;
        }
    }

    for (i = 0; i < MOTOR_NUM; i++) {
        motor_set((motor_id_t)i, (int16_t)u[i]);
    }
}
