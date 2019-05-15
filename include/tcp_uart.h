#ifndef _TCP_UART
#define _TCP_UART

#include "mgos.h"

#define UART0_SERVER_STRING "tcp://0.0.0.0:23"
#define UART1_SERVER_STRING "24"
#define UART2_SERVER_STRING "tcp://0.0.0.0:25"

int init_uart(int uart_index, int baud_rate, int data_bits, int parity, int stop_bits);
int init_uarts(bool uart0_enabled, bool uart1_enabled, bool uart2_enabled);

#endif
