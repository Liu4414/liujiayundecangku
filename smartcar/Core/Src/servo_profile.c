#include "servo_profile.h"

void servo_prof_init(servo_prof_t *p, int32_t pos, int32_t vmax, int32_t accel)
{
    p->pos    = pos;
    p->vel    = 0;
    p->target = pos;
    p->vmax   = vmax;
    p->accel  = accel;
}

void servo_prof_set_target(servo_prof_t *p, int32_t target)
{
    p->target = target;
}

uint8_t servo_prof_done(const servo_prof_t *p)
{
    return (uint8_t)((p->pos == p->target) && (p->vel == 0));
}

void servo_prof_update(servo_prof_t *p, int32_t dt_ms)
{
    int32_t err;
    int32_t dv;
    int32_t stop_dist;

    if ((p->accel <= 0) || (p->vmax <= 0)) {
        p->pos = p->target;
        p->vel = 0;
        return;
    }

    err = p->target - p->pos;
    if ((err == 0) && (p->vel == 0)) {
        return;
    }

    dv = p->accel * dt_ms / 1000;
    if (dv < 1) {
        dv = 1;
    }

    /* 以当前速度算“刹车距离”：v^2 / (2a) */
    stop_dist = (p->vel * p->vel) / (2 * p->accel);
    stop_dist += ((p->vel >= 0) ? p->vel : -p->vel) * dt_ms / 1000;

    if (err > 0) {
        if (err <= stop_dist) {
            p->vel -= dv;               /* 该减速了 */
        } else {
            p->vel += dv;               /* 继续加速 */
        }
        if (p->vel < 0)     p->vel = 0;
        if (p->vel > p->vmax) p->vel = p->vmax;
    } else {
        if (-err <= stop_dist) {
            p->vel += dv;
        } else {
            p->vel -= dv;
        }
        if (p->vel > 0)      p->vel = 0;
        if (p->vel < -p->vmax) p->vel = -p->vmax;
    }

    p->pos += p->vel * dt_ms / 1000;

    /* 防过冲 */
    if (((err > 0) && (p->pos > p->target)) ||
        ((err < 0) && (p->pos < p->target))) {
        p->pos = p->target;
        p->vel = 0;
        }
}
