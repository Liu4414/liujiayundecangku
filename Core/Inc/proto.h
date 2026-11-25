//
// Created by 52646 on 2026/11/20.
//

#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>

/* 12 字节帧（ESP32 esp32_ds4_bridge.ino 发出）
 * [0] AA [1] 55 [2] seq
 * [3] lx [4] ly [5] rx [6] ry
 * [7] btnL [8] btnH [9] l2 [10] r2
 * [11] crc8 对 [2]~[10] 共 9 字节
 */
#define PROTO_LEN     12
#define PROTO_HEAD0   0xAAu
#define PROTO_HEAD1   0x55u

typedef struct {
    uint8_t  seq;
    int8_t   lx;
    int8_t   ly;
    int8_t   rx;
    int8_t   ry;
    uint16_t btn;
    uint8_t  l2;
    uint8_t  r2;
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