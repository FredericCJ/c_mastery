#ifndef NR_CONGESTION_H
#define NR_CONGESTION_H
#include <stdint.h>

enum
{
    NR_OK,
    NR_WAIT,
    NR_INVALID,
    NR_BUSY,
    NR_FAST_RETRANSMIT,
    NR_STUB
};

enum
{
    NR_LIMIT = 1048576
};

struct nr_control
{
    uint32_t mss, iw, cwnd, ssthresh, rwnd, una, nxt, credit, duplicates;
    uint32_t recovery, recovery_limit, recovery_used, timeout_seen, timeout_una;
    uint32_t limited, quota, limited_start[2], limited_length[2];
    uint64_t now, last_send;
};

/* Established connection, external serialized header/tuple/ACK-window freshness
 * validation. This controller does not replace TCP parsing or byte ownership.
 */
int nr_init(struct nr_control *, uint32_t mss, uint32_t isn,
            uint32_t peer_window);
int nr_policy(struct nr_control *, unsigned limited_transmit);
uint32_t nr_flight(const struct nr_control *);
uint32_t nr_usable(const struct nr_control *);
int nr_send(struct nr_control *, uint32_t bytes, unsigned copied, uint64_t now,
            uint32_t *sequence);
int nr_ack(struct nr_control *, uint32_t ack, uint32_t window, unsigned payload,
           unsigned flags, uint64_t now);
int nr_timeout(struct nr_control *, uint64_t now);
int nr_restart(struct nr_control *, uint64_t now, uint32_t rto);
#endif
