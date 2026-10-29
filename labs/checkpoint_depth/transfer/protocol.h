#ifndef CHECKPOINT_TRANSFER_PROTOCOL_H
#define CHECKPOINT_TRANSFER_PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
#ifndef CPX_MAX_PAYLOAD
#define CPX_MAX_PAYLOAD 64
#endif
enum
{
    CPX_WIRE_MAX = CPX_MAX_PAYLOAD + 15
};

enum cpx_protocol_status
{
    CPX_P_OK,
    CPX_P_MORE,
    CPX_P_FRAME,
    CPX_P_REJECT,
    CPX_P_FULL,
    CPX_P_INVALID,
    CPX_P_NOT_IMPLEMENTED
};

struct cpx_message
{
    unsigned version, flags;
    uint16_t sequence;
    uint32_t stamp;
    size_t length;
    unsigned char payload[CPX_MAX_PAYLOAD];
};

struct cpx_parser
{
    unsigned char candidate[CPX_WIRE_MAX];
    size_t used, limit;
    unsigned ready;
    struct cpx_message message;
};

struct cpx_queue
{
    struct cpx_message items[4];
    size_t capacity, head, count;
};
enum cpx_protocol_status cpx_protocol_init(struct cpx_parser *, size_t);
enum cpx_protocol_status
cpx_protocol_feed(struct cpx_parser *, const unsigned char *, size_t, size_t *);
enum cpx_protocol_status cpx_protocol_queue(struct cpx_queue *, size_t);
enum cpx_protocol_status cpx_protocol_deliver(struct cpx_parser *,
                                              struct cpx_queue *);
enum cpx_protocol_status cpx_protocol_take(struct cpx_queue *,
                                           struct cpx_message *);
enum cpx_protocol_status cpx_protocol_encode(const struct cpx_message *,
                                             unsigned char *, size_t, size_t *);
enum cpx_protocol_status cpx_protocol_inplace(unsigned char *, size_t, size_t,
                                              unsigned, unsigned, uint16_t,
                                              uint32_t, size_t *);
#endif
