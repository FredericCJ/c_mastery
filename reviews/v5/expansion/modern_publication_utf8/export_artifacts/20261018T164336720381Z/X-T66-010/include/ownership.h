#ifndef CMASTERY_OWNERSHIP_H
#define CMASTERY_OWNERSHIP_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include "imports/v4_algo.h"
typedef enum { OW_OK, OW_INVALID, OW_FULL, OW_STALE, OW_BUSY, OW_NOMEM,
               OW_DEAD, OW_RETIRED, OW_NOT_IMPLEMENTED } OwStatus;
typedef struct { int32_t *block[3]; size_t count; V4Allocator allocator; } OwOwner;
OwStatus ow_construct(OwOwner *, V4Allocator, const size_t[3]);
OwStatus ow_destroy(OwOwner *);
typedef void (*OwCleanup)(void *);
typedef enum { OW_ALWAYS, OW_ON_ERROR, OW_ON_SUCCESS } OwWhen;
typedef struct { OwCleanup fn; void *context; OwWhen when; } OwCleanupEntry;
typedef struct { OwCleanupEntry entries[8]; size_t count; unsigned finishing; } OwScope;
OwStatus ow_defer(OwScope *, OwCleanup, void *, OwWhen);
OwStatus ow_finish(OwScope *, int success);
OwStatus ow_publish(OwOwner *published, OwOwner *staged);
OwStatus ow_move(OwOwner *destination, OwOwner *source);
#if defined(__GNUC__) || defined(__clang__)
#define OW_MUST_USE __attribute__((warn_unused_result))
#else
#define OW_MUST_USE
#endif
OW_MUST_USE OwStatus ow_consume(OwOwner *);
typedef struct { size_t offset, length; uint32_t serial; unsigned active, writer, retired; } OwLoan;
typedef struct { OwLoan loan[4]; size_t extent; } OwBorrows;
typedef struct { unsigned slot; uint32_t serial; } OwToken;
OwStatus ow_borrow(OwBorrows *, size_t, size_t, int writer, OwToken *);
OwStatus ow_release(OwBorrows *, OwToken);
OwStatus ow_split(OwBorrows *, OwToken parent, size_t left_length, OwToken children[2]);
typedef struct { int32_t value; OwBorrows borrows; } OwCell;
OwStatus ow_cell_read(const OwCell *, OwToken, int32_t *);
OwStatus ow_cell_write(OwCell *, OwToken, int32_t);
OwStatus ow_cell_replace(OwCell *, int32_t, int32_t *old);
typedef struct { unsigned strong, weak; OwCleanup destroy, release; void *context; } OwShared;
OwStatus ow_shared_retain(OwShared *);
OwStatus ow_shared_downgrade(OwShared *);
OwStatus ow_shared_upgrade(OwShared *);
OwStatus ow_shared_drop(OwShared *);
OwStatus ow_shared_weak_drop(OwShared *);
typedef struct { atomic_uint strong, weak; OwCleanup destroy, release; void *context; } OwAtomicShared;
OwStatus ow_atomic_retain(OwAtomicShared *);
OwStatus ow_atomic_downgrade(OwAtomicShared *);
OwStatus ow_atomic_upgrade(OwAtomicShared *);
OwStatus ow_atomic_drop(OwAtomicShared *);
OwStatus ow_atomic_weak_drop(OwAtomicShared *);
typedef struct { unsigned refs; int32_t *data; size_t length; V4Allocator allocator; } OwCow;
OwStatus ow_cow_clone(OwCow *);
OwStatus ow_cow_write(OwCow **owner, OwCow *fresh, size_t index, int32_t value);
OwStatus ow_cow_drop(OwCow *);
typedef struct { const void *address; uint64_t thread; unsigned pending, cancel; } OwPin;
OwStatus ow_pin_init(OwPin *, const void *, uint64_t);
OwStatus ow_pin_check(const OwPin *, const void *, uint64_t);
OwStatus ow_pin_submit(OwPin *, const void *, uint64_t);
OwStatus ow_pin_cancel(OwPin *, uint64_t);
OwStatus ow_pin_complete(OwPin *, uint64_t);
OwStatus ow_pin_rebind(OwPin *, const void *, uint64_t);
typedef struct {
 int32_t storage[8][16]; size_t used[8], attempts, fail_at, live, peak, releases;
 unsigned invalid_releases, exhausted_counters;
} OwAllocator;
V4Allocator ow_allocator(OwAllocator *);
/* Two synthetic tenants share one actual int32_t[16] object. */
typedef struct { int32_t data[16]; } OwTenantArena;
OwStatus ow_checked_write(OwTenantArena *, unsigned tenant, unsigned offset,
                          unsigned count, int32_t value);
OwStatus ow_flawed_write(OwTenantArena *, unsigned tenant, unsigned offset,
                         unsigned count, int32_t value);
#endif
