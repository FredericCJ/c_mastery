#ifndef EDU7_SESSION_SUCCESSOR_H
#define EDU7_SESSION_SUCCESSOR_H
#include "transport.h"

enum
{
    ES_CLOSED,
    ES_OPENING,
    ES_ACK_NEEDED,
    ES_ACKING,
    ES_READY
};

enum
{
    ES_DELIVERED = 30,
    ES_DUPLICATE = 31,
    ES_STUB = 99
};

struct es_peer
{
    struct e7_session core;
    struct et_tx tx;
    struct et_rx rx;
    unsigned self, peer, phase, uncertain, outgoing_kind, effects, responses;
    uint16_t sent_checkpoint, outgoing_checkpoint, outgoing_id, last_control,
        error_session, error_epoch;
    uint8_t sent[80], sent_length;
    int16_t setpoint;
    struct edu_record response;
};

struct es_event
{
    unsigned type, duplicate;
    uint16_t checkpoint;
    uint8_t reply[80];
    size_t length;
};

int es_init(struct es_peer *, unsigned, unsigned, uint16_t);
int es_emit(struct es_peer *, unsigned, uint16_t, uint16_t, uint16_t,
            const uint8_t *, size_t, uint16_t, uint32_t);
int es_offer(struct es_peer *, uint32_t, uint8_t *, size_t, size_t *,
             uint32_t *);
int es_admit(struct es_peer *, uint16_t, uint32_t, unsigned, uint32_t);
int es_ack(struct es_peer *, const uint8_t *, size_t, uint32_t);
int es_receive(struct es_peer *, const uint8_t *, size_t, uint32_t, uint8_t *,
               size_t, size_t *);
int es_deliver(struct es_peer *, size_t, struct es_event *);
int es_uncertain(struct es_peer *, uint16_t, uint16_t);
int es_permitted(const struct es_peer *);
int es_tests(unsigned);
#endif
