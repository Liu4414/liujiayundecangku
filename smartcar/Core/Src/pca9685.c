#include "config.h"
#include "i2c.h"
#include "pca9685.h"

#define REG_MODE1   0x00
#define REG_MODE2   0x01
#define REG_LED0    0x06
#define REG_ALL     0xFA
#define REG_PRE     0xFE

static uint8_t s_ready;

static uint16_t period_us(void)
{
    return (uint16_t)(1000000u / PCA9685_PWM_HZ);   /* 50Hz → 20000us */
}

static uint8_t pca_write(uint8_t reg, uint8_t val)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = val;

    if (HAL_I2C_Master_Transmit(&hi2c2,
                                (uint16_t)(PCA9685_I2C_ADDR << 1),
                                buf, 2, 20) != HAL_OK) {
        return 1;
    }
    return 0;
}

uint8_t pca9685_init(void)
{
    s_ready = 0;

    if (pca_write(REG_MODE2, 0x04)) return 0;   /* 推挽输出 */
    if (pca_write(REG_MODE1, 0x10)) return 0;   /* 睡眠，才能写预分频 */
    if (pca_write(REG_PRE, 121))    return 0;   /* 50Hz */
    if (pca_write(REG_MODE1, 0x00)) return 0;   /* 唤醒 */
    HAL_Delay(1);
    if (pca_write(REG_MODE1, 0xA0)) return 0;   /* RESTART + 地址自增 */

    s_ready = 1;
    return 1;
}

uint8_t pca9685_ready(void)
{
    return s_ready;
}

uint8_t pca9685_set_us(uint8_t ch, uint16_t us)
{
    uint8_t  buf[5];
    uint32_t cnt;
    uint32_t period;

    if ((!s_ready) || (ch > 15u)) {
        return 1;
    }

    period = (uint32_t)period_us();
    cnt = ((uint32_t)us * 4096u + period / 2u) / period;
    if (cnt > 4095u) {
        cnt = 4095u;
    }

    buf[0] = (uint8_t)(REG_LED0 + 4u * ch);
    buf[1] = 0x00;                                   /* ON_L */
    buf[2] = 0x00;                                   /* ON_H */
    buf[3] = (uint8_t)(cnt & 0xFFu);                 /* OFF_L */
    buf[4] = (uint8_t)((cnt >> 8) & 0x0Fu);          /* OFF_H */

    return (HAL_I2C_Master_Transmit(&hi2c2,
                                    (uint16_t)(PCA9685_I2C_ADDR << 1),
                                    buf, 5, 20) == HAL_OK) ? 0u : 1u;
}

uint8_t pca9685_set_all_us(uint16_t us)
{
    uint8_t  buf[5];
    uint32_t cnt;
    uint32_t period;

    if (!s_ready) {
        return 1;
    }

    period = (uint32_t)period_us();
    cnt = ((uint32_t)us * 4096u + period / 2u) / period;
    if (cnt > 4095u) {
        cnt = 4095u;
    }

    buf[0] = REG_ALL;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = (uint8_t)(cnt & 0xFFu);
    buf[4] = (uint8_t)((cnt >> 8) & 0x0Fu);

    return (HAL_I2C_Master_Transmit(&hi2c2,
                                    (uint16_t)(PCA9685_I2C_ADDR << 1),
                                    buf, 5, 20) == HAL_OK) ? 0u : 1u;
}
