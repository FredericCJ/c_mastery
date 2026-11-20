#ifndef BF_REPORT_H
#define BF_REPORT_H
/* Text reports over USART3 (ST-LINK virtual COM port) through the reused
 * reference driver (assessor/peripheral_depth/uart): polling backend,
 * at most eight bytes per submission, each submission completed at TC.
 *
 * Single foreground caller; no function here may run in handler mode.
 */
#include <stdint.h>

/* Configure PD8/PD9 and open the driver for 8N1 at `baud` on the 16 MHz
 * reset clock. Returns the pu_open status (0 on success). */
int bf_uart_open(uint32_t baud);

void bf_put(const char *text);
void bf_put_hex(uint32_t value);
void bf_put_dec(uint32_t value);
/* " name=0x........" and " name=decimal" fields. */
void bf_field_hex(const char *name, uint32_t value);
void bf_field_dec(const char *name, uint32_t value);
/* Append CR LF and transmit everything staged; returns after TC. */
void bf_line_end(void);
/* Transmit `count` copies of `value` with no framing text. */
void bf_put_repeated(uint8_t value, uint32_t count);

/* One bounded receive poll. Returns 1 and fills value/error (PE|FE|NF|ORE
 * bits) when a byte was taken, else 0. */
int bf_rx_poll(uint8_t *value, uint8_t *error);
/* Submissions that did not complete and forced a driver reopen. */
uint32_t bf_tx_failures(void);
#endif
