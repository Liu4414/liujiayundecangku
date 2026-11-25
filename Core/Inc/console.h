//
// Created by 52646 on 2026/11/20.
//

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

void console_init(void);
void console_rx_byte(uint8_t b);   /* USART1 接收中断里调用 */
void console_task(void);           /* 主循环里调用 */

#endif /* CONSOLE_H */
