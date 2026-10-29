#ifndef CMASTERY_PUBLICATION_H
#define CMASTERY_PUBLICATION_H
enum pf_publication_state { PF_STAGE_RETAINED, PF_VISIBLE_UNCONFIRMED, PF_SYNCED };
struct pf_synchronizer {
    void *context;
    int (*synchronize)(void *, int);
};
struct pf_publication { enum pf_publication_state state; int error; unsigned calls; };
/* Supplied publication mechanism: caller owns open stage/dir descriptors and
 * stage/target paths; it has finished writing stage. No descriptors are closed.
 * Before rename failure stage is retained. After rename, never claim rollback. */
struct pf_publication pf_publish(int stage, int directory, const char *stage_path,
                          const char *target_path, struct pf_synchronizer);
int pf_sync_system(void *, int);
#endif
