#include "recovery.h"
#include <limits.h>
#ifndef MC_MUTANT
#define MC_MUTANT 0
#endif
int mc_recovery_init(McRecovery *s, McResources ops) {
    if (!s || !ops.open || !ops.close) return 0;
    McRecovery value = {0}; value.resources = ops; *s = value; return 1;
}
int mc_recover(McRecovery *s, uint32_t token, unsigned *errors) {
    if (!s || !errors || !s->recovering || token != s->generation) return 0;
    unsigned failures = 0;
    for (unsigned i = 2; i-- > 0;) if (s->pending.mask & (1u << i)) {
        McClose r = s->resources.close(s->resources.context, s->pending.resource[i]);
        if (r != MC_RELEASED) ++failures;
        if ((r != MC_RETAINED || MC_MUTANT == 5) &&
            !(r == MC_RELEASED_ERROR && MC_MUTANT == 6))
            s->pending.mask &= ~(1u << i);
    }
    *errors = failures;
    if (s->pending.mask) return 0;
    (void)ow_destroy(&s->pending.cells);
    s->pending = (McFive){0}; s->recovering = 0; return 1;
}
static void retain(McRecovery *s, McFive value) {
    s->pending = value; s->recovering = 1; ++s->generation;
}
McRecovered mc_publish_recoverable(McRecovery *s, const unsigned char *p,
                                   size_t n, V4Allocator allocator) {
    McRecovered r = {0}; r.publication.storage = OW_INVALID;
    r.publication.parse = er_error(ER_INVALID, 0);
    if (!s || !s->resources.open || !s->resources.close || s->recovering ||
        s->generation >= UINT32_MAX-1) return r; /* reserve final retirement */
    McFive staged = {0};
    r.publication = mc_publish_pair(p, n, allocator, &staged.cells);
    if (!r.publication.committed) return r;
    r.publication.committed = false;
    for (unsigned i = 0; i < 2; ++i) {
        if (!s->resources.open(s->resources.context, i, &staged.resource[i])) {
            r.publication.storage = OW_NOMEM;
            retain(s, staged);
            r.recovery_token = s->generation;
            r.cleanup_pending = !mc_recover(s,s->generation,&r.close_errors);
            return r;
        }
        staged.mask |= 1u << i;
    }
    McFive old = s->current;
    s->current = staged; /* Exclusive-owner publication point. */
    r.publication.committed = true;
    retain(s, old);
    r.recovery_token = s->generation;
    r.cleanup_pending = !mc_recover(s,s->generation,&r.close_errors);
    return r;
}
int mc_retire_current(McRecovery *s, uint32_t *token) {
    if (!s || !token || s->recovering || s->generation == UINT32_MAX) return 0;
    retain(s, s->current); s->current = (McFive){0}; *token=s->generation; return 1;
}
