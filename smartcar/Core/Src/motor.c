#include "config.h"
#include "motor.h"

typedef struct {
    GPIO_TypeDef     *in1_port;
    uint16_t          in1_pin;
    GPIO_TypeDef     *in2_port;
    uint16_t          in2_pin;
    TIM_HandleTypeDef *htim;
    uint32_t          ch;
    uint8_t           invert;
} motor_hw_t;

static const motor_hw_t HW[MOTOR_NUM] = {
    { M1_IN1_PORT, M1_IN1_PIN, M1_IN2_PORT, M1_IN2_PIN, M1_PWM_HTIM, M1_PWM_CH, M1_INVERT },
    { M2_IN1_PORT, M2_IN1_PIN, M2_IN2_PORT, M2_IN2_PIN, M2_PWM_HTIM, M2_PWM_CH, M2_INVERT },
    { M3_IN1_PORT, M3_IN1_PIN, M3_IN2_PORT, M3_IN2_PIN, M3_PWM_HTIM, M3_PWM_CH, M3_INVERT },
    { M4_IN1_PORT, M4_IN1_PIN, M4_IN2_PORT, M4_IN2_PIN, M4_PWM_HTIM, M4_PWM_CH, M4_INVERT },
    { M5_IN1_PORT, M5_IN1_PIN, M5_IN2_PORT, M5_IN2_PIN, M5_PWM_HTIM, M5_PWM_CH, M5_INVERT },
    { M6_IN1_PORT, M6_IN1_PIN, M6_IN2_PORT, M6_IN2_PIN, M6_PWM_HTIM, M6_PWM_CH, M6_INVERT }
};

static int16_t s_speed[MOTOR_NUM];

/* ------------------------------------------------------------
 * 内部：把一路电机设置为 -1000 ~ +1000
 * ------------------------------------------------------------ */
static void motor_apply(motor_id_t id, int16_t permille)
{
    const motor_hw_t *h = &HW[id];
    int32_t  p = permille;
    uint32_t period;
    uint32_t ccr;

    if (h->invert) {
        p = -p;
    }

    if (p > 0) {
        /* 正转：IN1=1, IN2=0 */
        HAL_GPIO_WritePin(h->in1_port, h->in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(h->in2_port, h->in2_pin, GPIO_PIN_RESET);
    } else if (p < 0) {
        /* 反转：IN1=0, IN2=1 */
        HAL_GPIO_WritePin(h->in1_port, h->in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(h->in2_port, h->in2_pin, GPIO_PIN_SET);
        p = -p;
    } else {
        /* 0：两脚都低 = 滑行停止 */
        HAL_GPIO_WritePin(h->in1_port, h->in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(h->in2_port, h->in2_pin, GPIO_PIN_RESET);
    }

    period = (uint32_t)__HAL_TIM_GET_AUTORELOAD(h->htim) + 1u;   /* 900 */
    ccr    = (uint32_t)p * period / 1000u;

    if ((p > 0) && (ccr < (uint32_t)MOTOR_MIN_DUTY)) {
        ccr = (uint32_t)MOTOR_MIN_DUTY;
    }

    __HAL_TIM_SET_COMPARE(h->htim, h->ch, ccr);
}

/* ------------------------------------------------------------
 * 初始化：先关 STBY，清输出，启动 6 路 PWM，最后使能
 * ------------------------------------------------------------ */
void motor_init(void)
{
    uint8_t i;

    HAL_GPIO_WritePin(MOTOR_STBY_PORT, MOTOR_STBY_PIN, GPIO_PIN_RESET);

    for (i = 0; i < MOTOR_NUM; i++) {
        s_speed[i] = 0;
        motor_apply((motor_id_t)i, 0);
    }

    for (i = 0; i < MOTOR_NUM; i++) {
        HAL_TIM_PWM_Start(HW[i].htim, HW[i].ch);
    }

    HAL_Delay(1);
    HAL_GPIO_WritePin(MOTOR_STBY_PORT, MOTOR_STBY_PIN, GPIO_PIN_SET);
}

/* ------------------------------------------------------------
 * 总使能：0 = 切断所有电机（STBY 低）
 * ------------------------------------------------------------ */
void motor_enable(uint8_t on)
{
    uint8_t i;

    if (on) {
        HAL_GPIO_WritePin(MOTOR_STBY_PORT, MOTOR_STBY_PIN, GPIO_PIN_SET);
    } else {
        for (i = 0; i < MOTOR_NUM; i++) {
            s_speed[i] = 0;
            motor_apply((motor_id_t)i, 0);
        }
        HAL_GPIO_WritePin(MOTOR_STBY_PORT, MOTOR_STBY_PIN, GPIO_PIN_RESET);
    }
}

/* ------------------------------------------------------------
 * 设置单路转速：-1000 ~ +1000
 * ------------------------------------------------------------ */
void motor_set(motor_id_t id, int16_t permille)
{
    if (id >= MOTOR_NUM) {
        return;
    }

    if (permille >  1000) permille =  1000;
    if (permille < -1000) permille = -1000;

    s_speed[id] = permille;
    motor_apply(id, permille);
}

/* ------------------------------------------------------------
 * 单路滑行停止
 * ------------------------------------------------------------ */
void motor_stop(motor_id_t id)
{
    motor_set(id, 0);
}

/* ------------------------------------------------------------
 * 单路短路制动：IN1=IN2=1，PWM=0
 * ------------------------------------------------------------ */
void motor_brake(motor_id_t id)
{
    const motor_hw_t *h;

    if (id >= MOTOR_NUM) {
        return;
    }

    h = &HW[id];
    s_speed[id] = 0;

    HAL_GPIO_WritePin(h->in1_port, h->in1_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(h->in2_port, h->in2_pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(h->htim, h->ch, 0);
}

/* ------------------------------------------------------------
 * 六轮滑行停止
 * ------------------------------------------------------------ */
void motor_stop_all(void)
{
    uint8_t i;

    for (i = 0; i < MOTOR_NUM; i++) {
        motor_set((motor_id_t)i, 0);
    }
}

/* ------------------------------------------------------------
 * 读当前设定值
 * ------------------------------------------------------------ */
int16_t motor_get(motor_id_t id)
{
    if (id >= MOTOR_NUM) {
        return 0;
    }
    return s_speed[id];
}
