#ifndef AMP_FOLLOWTHROUGH_H
#define AMP_FOLLOWTHROUGH_H
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

enum
{
    AF_OK,
    AF_INVALID,
    AF_FULL,
    AF_EMPTY,
    AF_STALE,
    AF_BUSY,
    AF_EXHAUSTED,
    AF_CONFLICT,
    AF_UNIMPLEMENTED = 99
};

enum
{
    AF_CAPACITY = 16,
    AF_PAYLOAD = 32,
    AF_LAYOUT = 0x41463220u
};

/* Fixed prefix may be inspected before the payload layout is accepted.
 * Initial construction/reset requires explicit peer AND DMA quiescence.
 * One supervisor writes ready/layout/epoch; peer alone writes ack. */
struct af_boot
{
    _Atomic uint32_t ready, version, layout, epoch, ack, parked;
};

struct af_boot_owner
{
    uint32_t next_epoch, stage, start, timeout;
    unsigned active;
};

int af_boot_begin(struct af_boot_owner *, uint32_t, uint32_t, uint32_t,
                  unsigned, unsigned);
int af_boot_stage(struct af_boot_owner *, struct af_boot *, unsigned, uint32_t,
                  uint32_t);
int af_boot_ack(struct af_boot *, uint32_t, uint32_t, uint32_t);
int af_boot_close(struct af_boot_owner *, struct af_boot *, unsigned, unsigned);
int af_boot_status(const struct af_boot_owner *, const struct af_boot *,
                   uint32_t);
int af_elapsed(uint32_t, uint32_t, uint32_t, unsigned *);

struct af_message
{
    uint32_t epoch, sequence, length;
    unsigned char bytes[AF_PAYLOAD];
};

struct af_ring
{
    _Alignas(64) _Atomic uint32_t head;
    _Alignas(64) _Atomic uint32_t tail;
    struct af_message slots[AF_CAPACITY];
};

/* Exactly one producer and one consumer. No object copy/reset while active.
 * Caller input/output/control remain live, disjoint and exclusively owned. */
void af_ring_init(struct af_ring *, uint32_t);
int af_batch(struct af_ring *, uint32_t, const struct af_message *, unsigned);
int af_receive(struct af_ring *, uint32_t, void *, size_t, uint32_t *,
               size_t *);
uint32_t af_queued(const struct af_ring *);
int af_cache_span(uint32_t, uint32_t, uint32_t, uint32_t *, uint32_t *);

struct af_receipt
{
    uint32_t session, last_id;
    int32_t total, last_delta, last_result;
};

/* Serialized latest-only receipt in survivor-owned memory; peer reset can
 * preserve it, full-machine reset does not imply persistent retention. */
int af_effect(struct af_receipt *, uint32_t, uint32_t, int32_t, int32_t *);

struct af_bank
{
    uint32_t base, bytes;
    unsigned masters, domain;
};

int af_region(const struct af_bank *, size_t, uint32_t, uint32_t, unsigned,
              unsigned *);

struct af_owners
{
    unsigned clock_owner, dma_owner, telemetry_owner, compute_owner;
};

int af_owner_claim(struct af_owners *, unsigned, unsigned);
#endif
