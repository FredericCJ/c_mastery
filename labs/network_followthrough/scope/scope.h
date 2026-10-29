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
struct nw_lengths {
  uint32_t ip, unpadded, padding, pre_fcs, with_fcs, dma_bytes;
};
int nw_lengths(uint32_t payload, struct nw_lengths *);
struct nw_ticket {
  uint32_t epoch, owner, generation, serial;
};
struct nw_tx {
  unsigned char bytes[1600];
  uint32_t epoch, owner, generation, serial;
  uint16_t length, sent, acknowledged, prepared_length, flight_end;
  unsigned prepared, flight, dma;
};
struct nw_segment {
  unsigned char bytes[256];
  uint16_t length, offset;
  struct nw_ticket ticket;
};
int nw_tx_init(struct nw_tx *, uint32_t epoch, uint32_t connection_identity);
int nw_offer(struct nw_tx *, const void *, size_t);
int nw_prepare(struct nw_tx *, unsigned mss, unsigned peer_window, struct nw_segment *);
int nw_submit(struct nw_tx *, struct nw_ticket, unsigned accepted);
int nw_dma_done(struct nw_tx *, struct nw_ticket);
/* Validated remote ACK mapped to this connection/incarnation; serial unused.
 * This local token does not authenticate a network peer or validate TCP
 * headers. */
int nw_ack(struct nw_tx *, struct nw_ticket, uint32_t cumulative_bytes);
struct nw_rx {
  unsigned char bytes[512];
  uint16_t head, count, capacity;
};
int nw_rx_init(struct nw_rx *, unsigned capacity);
/* Atomic admission: OK copies all bytes; BUSY consumes none. Input expires
 * on return. Calls on one object are externally serialized. */
int nw_rx_offer(struct nw_rx *, const void *, size_t);
int nw_rx_peek(const struct nw_rx *, const unsigned char **, size_t *);
int nw_rx_consume(struct nw_rx *, size_t);
unsigned nw_rx_window(const struct nw_rx *);
#endif
