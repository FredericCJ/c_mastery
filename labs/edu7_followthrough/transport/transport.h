#ifndef EDU7_TRANSPORT_SUCCESSOR_H
#define EDU7_TRANSPORT_SUCCESSOR_H
#include "com_edu7.h"
#include "edu7_depth.h"

enum
{
    ET_COMPLETE = 20,
    ET_STUB = 99
};

struct et_identity
{
    uint16_t epoch;
    uint8_t profile, node, peer, port, peer_port;
};

struct et_tx
{
    struct et_identity identity;
    struct e7_sender inner;
    unsigned peer_credit;
};

struct et_rx
{
    struct et_identity identity;
    struct edu_rx inner;
    uint16_t retired;
    uint32_t last, started, expired;
};

int et_tx_open(struct et_tx *, unsigned, uint16_t, unsigned);
int et_rx_open(struct et_rx *, unsigned, uint16_t, unsigned);
int et_begin(struct et_tx *, uint16_t, const uint8_t *, size_t, uint32_t);
int et_offer(struct et_tx *, uint32_t, uint8_t *, size_t, size_t *, uint32_t *);
int et_accept(struct et_tx *, uint16_t, uint32_t, unsigned, uint32_t);
int et_ack(struct et_tx *, const uint8_t *, size_t, uint32_t);
int et_receive(struct et_rx *, const uint8_t *, size_t, uint32_t, uint8_t *,
               size_t, size_t *);
int et_expire(struct et_rx *, uint32_t);
int et_peek(const struct et_rx *, uint8_t *, size_t, size_t *);
int et_release(struct et_rx *, uint16_t, uint16_t);
int et_tests(unsigned);
#endif
