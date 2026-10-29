#ifndef CMASTERY_MC_RECOVERY_H
#define CMASTERY_MC_RECOVERY_H
#include "composition.h"
/* RETAINED means no release took place. RELEASED_ERROR is diagnostic only:
   retrying that identity is forbidden. Callbacks never reenter or longjmp. */
enum mc_close { MC_RELEASED, MC_RETAINED, MC_RELEASED_ERROR };
struct mc_resources {
    void *context;
    int (*open)(void *, unsigned ordinal, int *identity);
    enum mc_close (*close)(void *, int identity);
};
struct mc_five { struct ow_owner cells; int resource[2]; unsigned mask; };
struct mc_recovery {
    struct mc_five current, pending;
    struct mc_resources resources;
    uint32_t generation;
    unsigned recovering;
};
struct mc_recovered {
    struct mc_publish publication;
    uint32_t recovery_token;
    unsigned cleanup_pending, close_errors;
};
/* Initialize zero-owned storage before use. init transfers no resources. */
int mc_recovery_init(struct mc_recovery *, struct mc_resources);
struct mc_recovered mc_publish_recoverable(struct mc_recovery *, const unsigned char *, size_t,
                                  struct v4_allocator);
int mc_recover(struct mc_recovery *, uint32_t token, unsigned *close_errors);
int mc_retire_current(struct mc_recovery *, uint32_t *token);
#endif
