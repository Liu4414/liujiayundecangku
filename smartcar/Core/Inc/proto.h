//
// Created by 52646 on 2026/11/20.
//

#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>

/* 帧格式（18 字节）
 * [0]  0xA5   帧头1
 * [1]  0x5A   帧头2
 * [2]  seq
 * [3]  flags  bit0 急停  bit1 使能  bit2~7 保留
 * [4]  lx     -100~100 左摇杆X
 * [5]  ly     -100~100 左摇杆Y（前为正）
 * [6]  rx     -100~100 右摇杆X（右为正）
 * [7]  ry     -100~100 右摇杆Y
 * [8]  l2     0~255 左扳机
 * [9]  r2     0~255 右扳机
 * [10] s0     -100~100 舵机0
 * [11] s1
 * [12] s2
 * [13] s3
 * [14] s4
 * [15] btn_lo
 * [16] btn_hi
 * [17] crc8   对 [2]~[16] 共 15 字节
 */
#define PROTO_LEN      18
#define PROTO_HEAD0    0xA5u
#define PROTO_HEAD1    0x5Au
#define PROTO_AUX_NUM  5

typedef struct {
    uint8_t seq;
    uint8_t flags;
    int8_t  lx;
    int8_t  ly;
    int8_t  rx;
    int8_t  ry;
    uint8_t l2;
    uint8_t r2;
    int8_t  s[PROTO_AUX_NUM];
    uint8_t btn_lo;
    uint8_t btn_hi;
} rc_frame_t;

typedef enum {
    PROTO_NONE = 0,
    PROTO_FRAME,
    PROTO_CRC_ERR
} proto_result_t;

typedef struct {
    uint8_t buf[PROTO_LEN];
    uint8_t idx;
} proto_ctx_t;

void           proto_init(proto_ctx_t *c);
proto_result_t proto_feed(proto_ctx_t *c, uint8_t b, rc_frame_t *out);
uint8_t        proto_crc8(const uint8_t *data, uint16_t len);

#endif /* PROTO_H */