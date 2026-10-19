#ifndef CMASTERY_FAILURE_PUBLICATION_H
#define CMASTERY_FAILURE_PUBLICATION_H
#include "transaction.h"
/* Hosted concurrent changed-condition port. The owner outlives every lease.
 * One writer; any number of readers. No allocator or pthreads in transaction.c.
 */
typedef struct AePublication AePublication;
typedef struct AePublishedNode AePublishedNode;
typedef struct {
  AePublication *owner;
  AePublishedNode *node;
} AeReadLease;
AePublication *ae_publication_new(const AeConfig *);
AeCode ae_publication_replace(AePublication *, const AeConfig *,
                              unsigned fail_allocation);
AeCode ae_publication_acquire(AePublication *, AeReadLease *);
AeCode ae_publication_read(const AeReadLease *, AeConfig *);
AeCode ae_publication_release(AeReadLease *);
AeCode ae_publication_counts(AePublication *, unsigned *made, unsigned *freed,
                             unsigned *live);
AeCode ae_publication_close(AePublication *);
AeCode ae_publication_destroy(AePublication **);
#endif
