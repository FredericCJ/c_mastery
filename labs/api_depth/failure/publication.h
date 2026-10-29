#ifndef CMASTERY_FAILURE_PUBLICATION_H
#define CMASTERY_FAILURE_PUBLICATION_H
#include "transaction.h"
/* Hosted concurrent changed-condition port. The owner outlives every lease.
 * One writer; any number of readers. No allocator or pthreads in transaction.c.
 */
struct ae_publication;
struct ae_published_node;
struct ae_read_lease {
  struct ae_publication *owner;
  struct ae_published_node *node;
};
struct ae_publication *ae_publication_new(const struct ae_config *);
enum ae_code ae_publication_replace(struct ae_publication *, const struct ae_config *,
                              unsigned fail_allocation);
enum ae_code ae_publication_acquire(struct ae_publication *, struct ae_read_lease *);
enum ae_code ae_publication_read(const struct ae_read_lease *, struct ae_config *);
enum ae_code ae_publication_release(struct ae_read_lease *);
enum ae_code ae_publication_counts(struct ae_publication *, unsigned *made, unsigned *freed,
                             unsigned *live);
enum ae_code ae_publication_close(struct ae_publication *);
enum ae_code ae_publication_destroy(struct ae_publication **);
#endif
