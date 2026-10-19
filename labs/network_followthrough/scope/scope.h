#ifndef CMASTERY_NETWORK_SCOPE_H
#define CMASTERY_NETWORK_SCOPE_H
#include <stddef.h>
#include <stdint.h>
enum {
  NW_OK,
  NW_INVALID,
  NW_BUSY,
  NW_STALE,
  NW_RANGE,
  NW_WAIT,
  NW_DUPLICATE,
  NW_UNIMPLEMENTED = 99
};
typedef struct {
  uint32_t ip, unpadded, padding, pre_fcs, with_fcs, dma_bytes;
} NwLengths;
int nw_lengths(uint32_t payload, NwLengths *);
typedef struct {
  uint32_t epoch, owner, generation, serial;
} NwTicket;
typedef struct {
  unsigned char bytes[1600];
  uint32_t epoch, owner, generation, serial;
  uint16_t length, sent, acknowledged, prepared_length, flight_end;
  unsigned prepared, flight, dma;
} NwTx;
typedef struct {
  unsigned char bytes[256];
  uint16_t length, offset;
  NwTicket ticket;
} NwSegment;
int nw_tx_init(NwTx *, uint32_t epoch, uint32_t connection_identity);
int nw_offer(NwTx *, const void *, size_t);
int nw_prepare(NwTx *, unsigned mss, unsigned peer_window, NwSegment *);
int nw_submit(NwTx *, NwTicket, unsigned accepted);
int nw_dma_done(NwTx *, NwTicket);
/* Validated remote ACK mapped to this connection/incarnation; serial unused.
 * This local token does not authenticate a network peer or validate TCP
 * headers. */
int nw_ack(NwTx *, NwTicket, uint32_t cumulative_bytes);
typedef struct {
  unsigned char bytes[512];
  uint16_t head, count, capacity;
} NwRx;
int nw_rx_init(NwRx *, unsigned capacity);
/* Atomic admission: OK copies all bytes; BUSY consumes none. Input expires
 * on return. Calls on one object are externally serialized. */
int nw_rx_offer(NwRx *, const void *, size_t);
int nw_rx_peek(const NwRx *, const unsigned char **, size_t *);
int nw_rx_consume(NwRx *, size_t);
unsigned nw_rx_window(const NwRx *);
#endif
