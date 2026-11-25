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

/* 逻辑通道 i → PCA9685 物理通道 */
static const uint8_t s_map[SERVO_NUM] = SERVO_CH_LIST;

/* 每一路的行程（us） */
static const uint16_t s_min[SERVO_NUM] = SERVO_MIN_LIST;
static const uint16_t s_mid[SERVO_NUM] = SERVO_MID_LIST;
static const uint16_t s_max[SERVO_NUM] = SERVO_MAX_LIST;

void servo_init(void)
{
    uint8_t i;

    s_ok = pca9685_init();

    for (i = 0; i < SERVO_NUM; i++) {
        s_ch[i].invert  = 0;          /* 某一路反了改成 1 */
        s_ch[i].last_us = (int16_t)s_mid[i];

        servo_prof_init(&s_ch[i].prof, (int32_t)s_mid[i], SERVO_VMAX, SERVO_ACCEL);

        if (s_ok) {
            pca9685_set_us(s_map[i], s_mid[i]);
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
        us = (int32_t)s_mid[ch] +
             ((int32_t)pos * ((int32_t)s_max[ch] - (int32_t)s_mid[ch])) / 100;
    } else {
        us = (int32_t)s_mid[ch] +
             ((int32_t)pos * ((int32_t)s_mid[ch] - (int32_t)s_min[ch])) / 100;
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
            if (pca9685_set_us(s_map[i], (uint16_t)s_ch[i].prof.pos) == 0) {
                s_ch[i].last_us = (int16_t)s_ch[i].prof.pos;
            }
        }
    }
}

/* 直接按微秒设置目标（arm 层用），会夹到全局行程内 */
void servo_set_target_us(uint8_t ch, uint16_t us)
{
    if (ch >= SERVO_NUM) {
        return;
    }

    if (us < (int32_t)s_min[ch]) us = (int32_t)s_min[ch];
    if (us > (int32_t)s_max[ch]) us = (int32_t)s_max[ch];

    servo_prof_set_target(&s_ch[ch].prof, (int32_t)us);
}

int16_t servo_get_us(uint8_t ch)
{
    if (ch >= SERVO_NUM) {
        return -1;
    }
    return (int16_t)s_ch[ch].prof.pos;
}
