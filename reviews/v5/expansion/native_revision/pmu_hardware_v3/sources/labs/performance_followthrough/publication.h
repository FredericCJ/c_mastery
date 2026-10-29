#ifndef CMASTERY_PUBLICATION_H
#define CMASTERY_PUBLICATION_H
enum PFPublicationState { PF_STAGE_RETAINED, PF_VISIBLE_UNCONFIRMED, PF_SYNCED };
typedef struct {
    void *context;
    int (*synchronize)(void *, int);
} PFSynchronizer;
typedef struct { enum PFPublicationState state; int error; unsigned calls; } PFPublication;
/* Supplied publication mechanism: caller owns open stage/dir descriptors and
 * stage/target paths; it has finished writing stage. No descriptors are closed.
 * Before rename failure stage is retained. After rename, never claim rollback. */
PFPublication pf_publish(int stage, int directory, const char *stage_path,
                          const char *target_path, PFSynchronizer);
int pf_sync_system(void *, int);
#endif
