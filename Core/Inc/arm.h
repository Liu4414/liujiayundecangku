//
// Created by 52646 on 2026/11/20.
//

#ifndef ARM_H
#define ARM_H

#include "proto.h"

void arm_init(void);
void arm_update(const rc_frame_t *f);
void arm_home(void);

#endif /* ARM_H */
