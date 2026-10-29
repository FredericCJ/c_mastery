#ifndef B06_DURABLE_H
#define B06_DURABLE_H
#include <stddef.h>
typedef void (*du_hook_fn)(unsigned phase, void *context);
/* Fixed owned names source/tmp -> destination/target; same filesystem.
 * Return0 only after both directory syncs. phase reports completed boundary.
 * Inject failure immediately before phase1..5, or0 disabled (teaching only). */
int du_replace(int source, int destination, const unsigned char *, size_t,
               unsigned fail_before, unsigned *phase, du_hook_fn, void *);
#endif
