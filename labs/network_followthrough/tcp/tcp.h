#ifndef NC_TCP_H
#define NC_TCP_H
#include "ip.h"
#include "scope.h"
#include "stream.h"
enum {
  NC_OK,
  NC_WAIT,
  NC_INVALID,
  NC_BUSY,
  NC_STALE,
  NC_RANGE,
  NC_ABORT,
  NC_STUB
};
enum {
  NC_LISTEN,
  NC_SYN_RECEIVED,
  NC_ESTABLISHED,
  NC_FIN_WAIT_1,
  NC_FIN_WAIT_2,
  NC_CLOSING,
  NC_CLOSE_WAIT,
  NC_LAST_ACK,
  NC_TIME_WAIT,
  NC_CLOSED
};
enum { NC_NEW, NC_RETRY, NC_ACK, NC_PROBE };
typedef struct {
  uint32_t srtt, variance, rto, samples, granularity;
  unsigned shift;
} NcRtt;
typedef struct {
  uint32_t epoch, serial;
} NcTicket;
typedef struct {
  unsigned char bytes[300];
  size_t length;
  NcTicket ticket;
} NcPacket;
typedef struct {
  NiConfig config;
  NwRx rx;
  NcRtt rtt;
  NcPacket pending, retained;
  unsigned char tx[1600];
  size_t tx_length, tx_at;
  uint64_t now, sent_at, due, probe_due, timewait_until;
  uint32_t epoch, serial, dma_serial, isn, snd_una, snd_nxt, rcv_nxt, wl1, wl2;
  uint32_t rto, probe_rto;
  unsigned state, port, mss, peer_window, pending_valid, pending_kind;
  unsigned pending_span, pending_data, flight, retries, retransmitted;
  unsigned syn_lost, ack_needed, close_requested, peer_fin, dma, exhausted;
} NcStack;
int nc_rtt_init(NcRtt *, unsigned granularity, unsigned shift);
int nc_rtt_sample(NcRtt *, unsigned milliseconds, unsigned retransmitted);
int nc_init(NcStack *, NiConfig, uint32_t epoch, uint32_t isn,
            unsigned receive_capacity, unsigned estimator_shift);
/* All calls serialized, spans disjoint from state/output. No hidden allocation.
 * A fresh epoch requires prior lower-layer quiescence. See CONTRACTS.md. */
int nc_receive(NcStack *, const unsigned char *, size_t, uint64_t now);
int nc_tick(NcStack *, uint64_t now);
int nc_offer(NcStack *, const void *, size_t);
int nc_close(NcStack *);
/* Logical local abort; preserves the separately owned DMA ticket until actual
 * completion or positively established lower-layer quiescence. Idempotent. */
int nc_abort(NcStack *, uint64_t now);
int nc_prepare(NcStack *, NcPacket *);
int nc_submit(NcStack *, NcTicket, unsigned copied, uint64_t now);
int nc_dma_done(NcStack *, NcTicket);
int nc_consume(NcStack *, size_t);
int nc_http_pump(NcStack *, HttpParser *, size_t budget, size_t *used);
int nc_http_eof(NcStack *, HttpParser *);
unsigned nc_usable(const NcStack *);
#endif
