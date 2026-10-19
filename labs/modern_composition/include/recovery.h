#ifndef CMASTERY_MC_RECOVERY_H
#define CMASTERY_MC_RECOVERY_H
#include "composition.h"
/* RETAINED means no release took place. RELEASED_ERROR is diagnostic only:
   retrying that identity is forbidden. Callbacks never reenter or longjmp. */
typedef enum { MC_RELEASED, MC_RETAINED, MC_RELEASED_ERROR } McClose;
typedef struct {
    void *context;
    int (*open)(void *, unsigned ordinal, int *identity);
    McClose (*close)(void *, int identity);
} McResources;
typedef struct { OwOwner cells; int resource[2]; unsigned mask; } McFive;
typedef struct {
    McFive current, pending;
    McResources resources;
    uint32_t generation;
    unsigned recovering;
} McRecovery;
typedef struct {
    McPublish publication;
    uint32_t recovery_token;
    unsigned cleanup_pending, close_errors;
} McRecovered;
/* Initialize zero-owned storage before use. init transfers no resources. */
int mc_recovery_init(McRecovery *, McResources);
McRecovered mc_publish_recoverable(McRecovery *, const unsigned char *, size_t,
                                  V4Allocator);
int mc_recover(McRecovery *, uint32_t token, unsigned *close_errors);
int mc_retire_current(McRecovery *, uint32_t *token);
#endif
