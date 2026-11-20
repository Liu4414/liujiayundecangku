#include "config.h"
#include "servo.h"
#include "pca9685.h"
#include "servo_profile.h"

typedef struct {
    servo_prof_t prof;
    int16_t      last_us;
    uint8_t      invert;
} servo_ch_t;

static servo_ch_t s_ch[SERVO_NUM];
static uint8_t    s_ok;

void servo_init(void)
{
    uint8_t i;

    s_ok = pca9685_init();

    for (i = 0; i < SERVO_NUM; i++) {
        s_ch[i].invert  = 0;          /* 某一路反了改成 1 */
        s_ch[i].last_us = SERVO_MID_US;

        servo_prof_init(&s_ch[i].prof, SERVO_MID_US, SERVO_VMAX, SERVO_ACCEL);

        if (s_ok) {
            pca9685_set_us(i, SERVO_MID_US);
        }
    }
}

uint8_t servo_ok(void)
{
    return s_ok;
}

void servo_set_norm(uint8_t ch, int16_t pos)
{
    int32_t us;

    if (ch >= SERVO_NUM) {
        return;
    }

    if (pos >  100) pos =  100;
    if (pos < -100) pos = -100;

    if (s_ch[ch].invert) {
        pos = -pos;
    }

    if (pos >= 0) {
        us = SERVO_MID_US +
             ((int32_t)pos * (SERVO_MAX_US - SERVO_MID_US)) / 100;
    } else {
        us = SERVO_MID_US +
             ((int32_t)pos * (SERVO_MID_US - SERVO_MIN_US)) / 100;
    }

    servo_prof_set_target(&s_ch[ch].prof, us);
}

void servo_task_10ms(void)
{
    uint8_t i;

    if (!s_ok) {
        return;
    }

    for (i = 0; i < SERVO_NUM; i++) {
        servo_prof_update(&s_ch[i].prof, 10);

        if (s_ch[i].prof.pos != (int32_t)s_ch[i].last_us) {
            if (pca9685_set_us(i, (uint16_t)s_ch[i].prof.pos) == 0) {
                s_ch[i].last_us = (int16_t)s_ch[i].prof.pos;
            }
        }
    }
}
