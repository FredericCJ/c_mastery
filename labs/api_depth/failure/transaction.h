#ifndef CMASTERY_FAILURE_TRANSACTION_H
#define CMASTERY_FAILURE_TRANSACTION_H
#include "public_error.h"

struct ae_config
{
    unsigned values[3], count, version;
};

enum ae_guarantee
{
    AE_STRONG,
    AE_BASIC,
    AE_PARTIAL
};

struct ae_update
{
    enum ae_code code;
    unsigned committed, accepted;
};

typedef void (*ae_observe_fn)(void *, const struct ae_config *);
/* fail=0 none;1 validation boundary;2 staging;3..n+2 before copy;
 * n+3 before commit. Valid inputs <=9; capacity,n<=3. */
struct ae_update ae_replace(struct ae_config *, const unsigned *, unsigned n,
                            unsigned capacity, enum ae_guarantee, unsigned fail,
                            unsigned notify_failed, ae_observe_fn, void *);

struct ae_done
{
    unsigned id, delta;
};

struct ae_journal
{
    struct ae_done done[2];
    unsigned count, next, pending_id, pending_delta, effects, value;
};
/* crash=1 after intent/before effect;2 after effect/before done;
 * 3 after done/before acknowledgment;0 uninterrupted. */
enum ae_code ae_journal(struct ae_journal *, unsigned id, unsigned delta,
                        unsigned crash);
#endif
