#include "proto.h"

void proto_init(proto_ctx_t *c)
{
    c->idx = 0;
}

/* CRC-8：多项式 0x07，初值 0x00 */
uint8_t proto_crc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;
    uint8_t i;

    while (len--) {
        crc ^= *data++;
        for (i = 0; i < 8; i++) {
            if (crc & 0x80u) {
                crc = (uint8_t)((crc << 1) ^ 0x07u);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

proto_result_t proto_feed(proto_ctx_t *c, uint8_t b, rc_frame_t *out)
{
    /* 等帧头1 */
    if (c->idx == 0) {
        if (b == PROTO_HEAD0) {
            c->buf[0] = b;
            c->idx = 1;
        }
        return PROTO_NONE;
    }

    /* 等帧头2 */
    if (c->idx == 1) {
        if (b == PROTO_HEAD1) {
            c->buf[1] = b;
            c->idx = 2;
        } else if (b != PROTO_HEAD0) {
            c->idx = 0;
        }
        return PROTO_NONE;
    }

    /* 数据区里又出现 A5 5A：丢掉半帧，重新同步 */
    if ((c->buf[c->idx - 1] == PROTO_HEAD0) && (b == PROTO_HEAD1)) {
        c->buf[0] = PROTO_HEAD0;
        c->buf[1] = PROTO_HEAD1;
        c->idx = 2;
        return PROTO_NONE;
    }

    c->buf[c->idx++] = b;

    if (c->idx < PROTO_LEN) {
        return PROTO_NONE;
    }

    c->idx = 0;   /* 收满就复位，不管对错 */

    if (proto_crc8(&c->buf[2], 15) != c->buf[17]) {
        return PROTO_CRC_ERR;
    }

    out->seq    = c->buf[2];
    out->flags  = c->buf[3];
    out->lx     = (int8_t)c->buf[4];
    out->ly     = (int8_t)c->buf[5];
    out->rx     = (int8_t)c->buf[6];
    out->ry     = (int8_t)c->buf[7];
    out->l2     = c->buf[8];
    out->r2     = c->buf[9];
    out->s[0]   = (int8_t)c->buf[10];
    out->s[1]   = (int8_t)c->buf[11];
    out->s[2]   = (int8_t)c->buf[12];
    out->s[3]   = (int8_t)c->buf[13];
    out->s[4]   = (int8_t)c->buf[14];
    out->btn_lo = c->buf[15];
    out->btn_hi = c->buf[16];

    return PROTO_FRAME;
}
