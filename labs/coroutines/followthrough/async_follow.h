#ifndef CMASTERY_ASYNC_FOLLOW_H
#define CMASTERY_ASYNC_FOLLOW_H
#include "coroutines.h"
#include <pthread.h>
struct ca_frame;

struct ca_ticket
{
    struct ca_frame *owner;
    uint32_t generation, serial;
    unsigned slot;
};

struct ca_frame
{
    pthread_mutex_t mutex;
    struct co_allocator allocator;
    uint32_t generation, next;
    unsigned wanted, length, state, closing, ack, published, inflight;
    uint32_t tickets[4];
    unsigned char data[8];
};

int ca_create(struct co_allocator, unsigned, uint32_t, struct ca_frame **);
int ca_reserve(struct ca_frame *, struct ca_ticket *);
int ca_notify(struct ca_ticket *, const unsigned char *, size_t);
int ca_cancel(struct ca_frame *);
int ca_ack(struct ca_frame *);
int ca_poll(struct ca_frame *, unsigned char *, size_t *);
int ca_destroy(struct ca_frame **);
#endif
