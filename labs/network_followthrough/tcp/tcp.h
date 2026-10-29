#ifndef NC_TCP_H
#define NC_TCP_H
#include "ip.h"
#include "scope.h"
#include "stream.h"

enum
{
    NC_OK,
    NC_WAIT,
    NC_INVALID,
    NC_BUSY,
    NC_STALE,
    NC_RANGE,
    NC_ABORT,
    NC_STUB
};

enum
{
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

enum
{
    NC_NEW,
    NC_RETRY,
    NC_ACK,
    NC_PROBE
};

struct nc_rtt
{
    uint32_t srtt, variance, rto, samples, granularity;
    unsigned shift;
};

struct nc_ticket
{
    uint32_t epoch, serial;
};

struct nc_packet
{
    unsigned char bytes[300];
    size_t length;
    struct nc_ticket ticket;
};

struct nc_stack
{
    struct ni_config config;
    struct nw_rx rx;
    struct nc_rtt rtt;
    struct nc_packet pending, retained;
    unsigned char tx[1600];
    size_t tx_length, tx_at;
    uint64_t now, sent_at, due, probe_due, timewait_until;
    uint32_t epoch, serial, dma_serial, isn, snd_una, snd_nxt, rcv_nxt, wl1,
        wl2;
    uint32_t rto, probe_rto;
    unsigned state, port, mss, peer_window, pending_valid, pending_kind;
    unsigned pending_span, pending_data, flight, retries, retransmitted;
    unsigned syn_lost, ack_needed, close_requested, peer_fin, dma, exhausted;
};

int nc_rtt_init(struct nc_rtt *, unsigned granularity, unsigned shift);
int nc_rtt_sample(struct nc_rtt *, unsigned milliseconds,
                  unsigned retransmitted);
int nc_init(struct nc_stack *, struct ni_config, uint32_t epoch, uint32_t isn,
            unsigned receive_capacity, unsigned estimator_shift);
/* All calls serialized, spans disjoint from state/output. No hidden allocation.
 * A fresh epoch requires prior lower-layer quiescence. See CONTRACTS.md. */
int nc_receive(struct nc_stack *, const unsigned char *, size_t, uint64_t now);
int nc_tick(struct nc_stack *, uint64_t now);
int nc_offer(struct nc_stack *, const void *, size_t);
int nc_close(struct nc_stack *);
/* Logical local abort; preserves the separately owned DMA ticket until actual
 * completion or positively established lower-layer quiescence. Idempotent. */
int nc_abort(struct nc_stack *, uint64_t now);
int nc_prepare(struct nc_stack *, struct nc_packet *);
int nc_submit(struct nc_stack *, struct nc_ticket, unsigned copied,
              uint64_t now);
int nc_dma_done(struct nc_stack *, struct nc_ticket);
int nc_consume(struct nc_stack *, size_t);
int nc_http_pump(struct nc_stack *, struct http_parser *, size_t budget,
                 size_t *used);
int nc_http_eof(struct nc_stack *, struct http_parser *);
unsigned nc_usable(const struct nc_stack *);
#endif
