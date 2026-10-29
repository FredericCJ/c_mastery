#ifndef CMASTERY_VM_SERVICES_H
#define CMASTERY_VM_SERVICES_H
#include "store.h"

enum
{
    VH_LEGACY4,
    VH_VERSIONED5
};

struct vg_vh
{
    store *database;
    uint32_t capabilities;
    unsigned mode, allow_legacy;
    uint64_t operation_id;
};

/* Existing CvmHost ABI. Service1 assignment,2 read,3 increment. This adapter
 * owns no storage; caller serializes it with exclusive store access and keeps
 * configuration stable during each call. The native caller supplies one
 * durable logical operation ID before service3; the guest integer is delta.
 * Capabilities are checked before any backend operation. They are not native
 * memory isolation. Failure preserves output but may have committed effects.
 */
int vh_host(void *, uint8_t, int32_t, int32_t *);
/* Explicit idempotent migration of vm-result from four LE bytes to tag1+four
 * LE bytes. Requires VERSIONED5 and allow_legacy1; unknown formats refuse.
 * The underlying store format is unchanged. */
int vh_migrate(struct vg_vh *, int32_t *);
/* One-client strictly sequential nonzero64-bit IDs; latest duplicate with same
 * delta returns its saved result without reapplying. Older IDs, gaps, changed
 * payload and wrap refuse. vm-result and vm-operation receipt commit
 * atomically. This bounded receipt is not a general distributed exactly-once
 * guarantee.
 */
int vh_increment(struct vg_vh *, uint64_t, int32_t, int32_t *);
#endif
