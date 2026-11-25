//
// Created by 52646 on 2026/11/20.
//

#ifndef PCA9685_H
#define PCA9685_H

#include <stdint.h>

uint8_t pca9685_init(void);                       /* 1=成功，0=器件无应答 */
uint8_t pca9685_ready(void);
uint8_t pca9685_set_us(uint8_t ch, uint16_t us);  /* 0=成功 */
uint8_t pca9685_set_all_us(uint16_t us);

#endif /* PCA9685_H */
