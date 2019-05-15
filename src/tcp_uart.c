#include <stdio.h>

#include "mgos.h"
#include "mgos_uart.h"

#include "tcp_uart.h"

static struct mbuf uart0_rx_buf = {0};
static struct mbuf uart1_rx_buf = {0};
static struct mbuf uart2_rx_buf = {0};

/*
 * Note
 * The esp32 device has three uarts. Uart 0 is normally used for debugging.
 * The esp8266 device has two uarts, Uart 1 TX is normally used for debugging (no Uart 1 RX is available on the esp8266.
 *
 * For ESP32
 *
 * Uart 0
 * RX  = GPIO3
 * TX  = GPIO1
 * CTS = GPIO19
 * RTS = GPIO22
 *
 * Uart 1
 * RX  = GPIO25
 * TX  = GPIO26
 * CTS = GPIO27
 * RTS = GPIO13
 *
 * Uart 2
 * RX  = GPIO16
 * TX  = GPIO17
 * CTS = GPIO14
 * RTS = GPIO15
 */

/*
 * Dispatcher can be invoked with any amount of data (even none at all) and
 * at any time. Here we demonstrate how to process input line by line.
 */
static void uart_dispatcher(int uart_index, void *arg) {
  static struct mbuf rx_buf = {0};

  size_t rx_byte_count = mgos_uart_read_avail(uart_index);

  if (rx_byte_count == 0) {
	  return;
  }

  mgos_uart_read_mbuf(uart_index, &rx_buf, rx_byte_count);

  //If we have network connections then send the data
  //received on the uart port over the network
  if( uart_index == 0 ) {
	  mbuf_append(&uart0_rx_buf, rx_buf.buf, rx_buf.len);
  }
  else if( uart_index == 1 ) {
	  mbuf_append(&uart1_rx_buf, rx_buf.buf, rx_buf.len);
  }
  else if( uart_index == 2 ) {
	  mbuf_append(&uart2_rx_buf, rx_buf.buf, rx_buf.len);
  }

  /* Finally, remove the data from the buffer. */
  mbuf_remove(&rx_buf, rx_buf.len);

  (void) arg;
}

/**
 * @brief Initialise the uart.
 * @param uart_index The zero based index identifying the uart to be initialised.
 * @param baud_rate The required baud rate in bps.
 * @param data_bits The data bits.
 * @param parity The parity ( MGOS_UART_PARITY_NONE, MGOS_UART_PARITY_EVEN or MGOS_UART_PARITY_ODD )
 * @param stop_bits (  MGOS_UART_STOP_BITS_1 , MGOS_UART_STOP_BITS_2 or MGOS_UART_STOP_BITS_1_5)
 * @preturn 0 on success, -2 on failure.
 */
int init_uart(int uart_index, int baud_rate, int data_bits, enum parity, enum stop_bits) {
	  struct mgos_uart_config ucfg;

	  mgos_uart_config_set_defaults(uart_index, &ucfg);

	  /*
	   * At this point it is possible to adjust baud rate, pins and other settings.
	   * 115200 8-N-1 is the default mode, but we set it anyway
	   */
	  ucfg.baud_rate = 115200;
	  ucfg.num_data_bits = 8;
	  ucfg.parity = MGOS_UART_PARITY_NONE;
	  ucfg.stop_bits = MGOS_UART_STOP_BITS_1;
	  if (!mgos_uart_configure(uart_index, &ucfg)) {
	    return -1;
	  }

	  mgos_uart_set_dispatcher(uart_index, uart_dispatcher, NULL /* arg */);
	  mgos_uart_set_rx_enabled(uart_index, true);

	  return 0;
}

/**
 * @brief Show uart buf debug.
 */
/*
static void dbg_net_rx(int uart_index, struct mbuf *io) {

	char *nl = (char *) mg_strchr(mg_mk_str_n(io->buf, io->len), '\n');
	if (nl == NULL) return;
	*nl = '\0';
	size_t llen = nl - io->buf;
	struct mg_str line = mg_mk_str_n(io->buf, llen);
	LOG(LL_INFO, ("PJA: UART%d> '%.*s'", 0, (int) line.len, line.p));

}
*/

/**
 * @brief A handler to handle data received from the network and destined to be sent out of uart 0.
 * @param nc
 * @param ev
 * @param ev_data
 * @param user_data
 */
static void uart_0_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data) {
	struct mbuf *io = &nc->recv_mbuf;

	switch (ev) {
	case MG_EV_RECV:
		//Send the data out of the uart port
		mgos_uart_write(0, io->buf, io->len);
		mbuf_remove(io, io->len);      // Discard data from recv buffer
		break;
	default:
		break;
	}

	//Send data back if we have data to send
	if( uart0_rx_buf.len > 0 ) {
		mg_send(nc, uart0_rx_buf.buf, uart0_rx_buf.len);
	    mbuf_remove(&uart0_rx_buf, uart0_rx_buf.len);
	}

	(void) nc;
	(void) ev;
	(void) ev_data;
	(void) user_data;
}

/**
 * @brief A handler to handle data received from the network and destined to be sent out of uart 1.
 * @param nc
 * @param ev
 * @param ev_data
 * @param user_data
 */
static void uart_1_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data) {
	struct mbuf *io = &nc->recv_mbuf;

	switch (ev) {
	case MG_EV_RECV:
		//Send the data out of the uart port
		mgos_uart_write(1, io->buf, io->len);
		mbuf_remove(io, io->len);      // Discard data from recv buffer
		break;
	default:
		break;
	}

	//Send data back if we have data to send
	if( uart1_rx_buf.len > 0 ) {
		mg_send(nc, uart1_rx_buf.buf, uart1_rx_buf.len);
	    mbuf_remove(&uart1_rx_buf, uart1_rx_buf.len);
	}

	(void) nc;
	(void) ev;
	(void) ev_data;
	(void) user_data;
}

/**
 * @brief A handler to handle data received from the network and destined to be sent out of uart 2.
 * @param nc
 * @param ev
 * @param ev_data
 * @param user_data
 */
static void uart_2_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data) {
	struct mbuf *io = &nc->recv_mbuf;

	switch (ev) {
	case MG_EV_RECV:
		//Send the data out of the uart port
		mgos_uart_write(2, io->buf, io->len);
		mbuf_remove(io, io->len);      // Discard data from recv buffer
		break;
	default:
		break;
	}

	//Send data back if we have data to send
	if( uart2_rx_buf.len > 0 ) {
		mg_send(nc, uart2_rx_buf.buf, uart2_rx_buf.len);
	    mbuf_remove(&uart2_rx_buf, uart2_rx_buf.len);
	}

	(void) nc;
	(void) ev;
	(void) ev_data;
	(void) user_data;
}

/**
 * @brief init the UARTs to be used.
 * @param bool uart0_enable If true then UART 0 is enabled.
 * @param bool uart1_enable If true then UART 1 is enabled.
 * @param bool uart2_enable If true then UART 2 is enabled.
 * @return 0 if all requested UART initialisations completed successfully.
 *
 */
int init_uarts(bool uart0_enabled, bool uart1_enabled, bool uart2_enabled) {
	int rc = 0;
	struct mg_bind_opts bind_opts;

	memset(&bind_opts, 0, sizeof(bind_opts));

	if( uart0_enabled ) {
		rc = init_uart(0);
		mg_bind_opt(mgos_get_mgr(), UART0_SERVER_STRING, uart_0_handler, NULL, bind_opts);
	}

	if( uart1_enabled ) {
		if( !rc ) {
			rc = init_uart(1);
			mg_bind_opt(mgos_get_mgr(), UART1_SERVER_STRING, uart_1_handler, NULL, bind_opts);
		}
	}

	if( uart2_enabled ) {
		if( !rc ) {
			rc = init_uart(2);
			mg_bind_opt(mgos_get_mgr(), UART2_SERVER_STRING, uart_2_handler, NULL, bind_opts);
		}
	}

	return rc;
}
