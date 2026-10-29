#ifndef AF_BUFFERS_H
#define AF_BUFFERS_H
#include "amp.h"
#include "bridge.h"

enum
{
    AF_FREE,
    AF_FILLING,
    AF_DEVICE,
    AF_COMPLETED,
    AF_CONSUMING
};

struct af_buffer
{
    struct xd_lease lease;
    struct xd_ticket ticket;
    struct xd_result result;
    unsigned phase;
};

struct af_buffers
{
    struct pd_transfer transfer;
    struct af_buffer slot[2];
    unsigned active;
};

/* Serialized owner of one hardware stream and two disjoint whole-line buffers.
 * Initialization requires a trusted cold or externally fenced device. */
int af_buffers_init(struct af_buffers *, const struct xd_fence *);
int af_fill(struct af_buffers *, unsigned);
int af_submit(struct af_buffers *, struct pd_io, unsigned,
              const struct pd_extent *);
int af_timeout(struct af_buffers *, struct pd_io);
int af_complete(struct af_buffers *, struct pd_io);
int af_consume(struct af_buffers *, unsigned);
int af_release(struct af_buffers *, unsigned);
#endif
