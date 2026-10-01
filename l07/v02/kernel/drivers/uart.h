#ifndef _DRIVERS_UART_H
#define _DRIVERS_UART_H

#include "memlayout.h"
#include "types.h"

// PL011 UART (p. 175ff, §13; note errata)
#define UART_BASE (PERIPHERALS_BASE + 0x201000)
#define UART_DR (UART_BASE + 0x0)     // data register
#define UART_FR (UART_BASE + 0x18)    // flags register
#define UART_IBRD (UART_BASE + 0x24)  // integer baud rate divisor
#define UART_FBRD (UART_BASE + 0x28)  // fractional baud rate divisor
#define UART_LCRH (UART_BASE + 0x2c)  // line control register
#define UART_CR (UART_BASE + 0x30)    // control register
#define UART_IMSC (UART_BASE + 0x38)  // interrupt mask set clear register

void uart_init();
char uart_recv(void);
void uart_send(char c);
void uart_send_string(const char* s);

#endif  // _DRIVERS_UART_H
