#include "config.h"
#include "app.h"
#include "chassis.h"
#include "servo.h"

static rc_frame_t s_isr_frame;
static volatile uint32_t s_last_ms;
static volatile uint8_t  s_frame_new;
static uint8_t           s_have_frame;
static uint8_t           s_manual;

static rc_frame_t  s_frame;
static app_state_t s_state;

void app_init(void)
{
    s_last_ms    = 0;
    s_frame_new  = 0;
    s_have_frame = 0;
    s_manual     = 0;
    s_state      = APP_FAILSAFE;
}

void app_set_manual(uint8_t on)
{
    s_manual = on;
    if (on) {
        chassis_stop();
    }
}

void app_on_frame(const rc_frame_t *f, uint32_t now)
{
    s_isr_frame = *f;
    s_last_ms   = now;
    s_frame_new = 1;
}

void app_task_10ms(uint32_t now)
{
    uint8_t i;

    if (s_frame_new) {
        __disable_irq();
        s_frame = s_isr_frame;
        s_frame_new = 0;
        __enable_irq();
        s_have_frame = 1;
    }

    /* 手动模式：命令直接控制电机，app 不再干预 */
    if (s_manual) {
        servo_task_10ms();
        return;
    }

    if ((!s_have_frame) ||
        ((uint32_t)(now - s_last_ms) > APP_TIMEOUT_MS)) {
        chassis_stop();
        s_state = APP_FAILSAFE;
        } else if (s_frame.flags & 0x01u) {
            chassis_stop();
            s_state = APP_ESTOP;
        } else if (s_frame.l2 > 128u) {
            chassis_stop();
            s_state = APP_BRAKE;
        } else {
            chassis_set((int16_t)(APP_VX_SIGN * (int16_t)s_frame.ly * 10),
                        (int16_t)(APP_VY_SIGN * (int16_t)s_frame.lx * 10),
                        (int16_t)(APP_W_SIGN  * (int16_t)s_frame.rx * 10));
            chassis_task_10ms();

            for (i = 0; i < SERVO_NUM; i++) {
                servo_set_norm(i, s_frame.s[i]);
            }
            s_state = APP_RUN;
        }

    servo_task_10ms();
}

app_state_t app_state(void)
{
    return s_state;
}