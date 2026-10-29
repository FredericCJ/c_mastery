#ifndef CMASTERY_RATIONAL_OPAQUE_H
#define CMASTERY_RATIONAL_OPAQUE_H
#include "errors.h"
struct efr_opaque;

/* Hosted comparison only: raw suitably aligned distinct storage, not the T66
   typed-cell API. Callback/context lives until drop, no reentry/nonlocal exit. */
struct efr_memory
{
    void *context;
    void *(*take)(void *, size_t);
    void (*release)(void *, void *);
};

er_code efr_opaque_make(struct efr_memory, uint32_t, uint32_t,
                        struct efr_opaque **empty_owner);
er_code efr_opaque_consume(const struct efr_opaque *, uint32_t *);
void efr_opaque_drop(struct efr_opaque **owner);
#endif
