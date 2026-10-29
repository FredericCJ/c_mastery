#ifndef ETHERNET_RING_VARIANTS_H
#define ETHERNET_RING_VARIANTS_H
#include "ring.h"

/* Separate changed-condition models; not the selected hardware ring format. */
struct ev_chain
{
    const unsigned char *borrow;
    size_t length;
    uint32_t token;
    unsigned count, prepared, owned, aborting;
};

int ev_prepare(struct ev_chain *, const unsigned char *, size_t, unsigned count,
               uint32_t token);
int ev_own(struct ev_chain *, unsigned index);
int ev_complete(struct ev_chain *, unsigned returned, unsigned error);
int ev_abort(struct ev_chain *, unsigned stopped);
/* Stored bytes have already been validated and copied from a declared capture
 * boundary. This does not infer changed-policy RDES0 FL from a guessed value. */
int ev_span(const unsigned char *, size_t stored, unsigned cstf,
            size_t *exposed);
int ev_order(const unsigned char events[8], uint32_t *violated_edges);

struct ev_visibility
{
    unsigned char cpu[64], device[64];
    unsigned dirty, device_owned;
};

int ev_store(struct ev_visibility *, unsigned index, unsigned char value);
int ev_clean(struct ev_visibility *, unsigned line);
int ev_publish(struct ev_visibility *, unsigned line);
int ev_device_store(struct ev_visibility *, unsigned index,
                    unsigned char value);
int ev_return(struct ev_visibility *, unsigned line, unsigned returned);
int ev_invalidate(struct ev_visibility *, unsigned line);
#endif
