# TCP UART
Provide access to UARTs on ESP32/ESP8266 devices via a TCP connection.

## Introduction

The ~ESP32 and ESP8266 devices have serial ports
ESP32
 3 serial ports
ESP8266
 2 serial port. The second only has a TXD and is normally used for debugging.


### API

The init_uart() function is called to set the serial port parameters for a
single serial port.

The init_uarts() function is called in order to expose one or more
serial ports on TCP connections.

The first serial port (0) is reachable on TCP port 23.
The second serial port (1)  is reachable on TCP port 24.
The third serial port (1)  is reachable on TCP port 25.

### Example Application

```
#include "mgos.h"
#include "tcp_uart.h"

enum mgos_app_init_result mgos_app_init(void) {

    init_uart(1, 9600, 8, MGOS_UART_PARITY_NONE, MGOS_UART_STOP_BITS_1);
    //Expose UART1 on TCP port 24
    init_uarts(false, true, false);

    return MGOS_APP_INIT_SUCCESS;
}
```
