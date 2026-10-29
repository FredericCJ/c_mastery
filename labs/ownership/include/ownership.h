#ifndef CMASTERY_OWNERSHIP_H
#define CMASTERY_OWNERSHIP_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include "imports/v4_algo.h"
enum ow_status { OW_OK, OW_INVALID, OW_FULL, OW_STALE, OW_BUSY, OW_NOMEM,
               OW_DEAD, OW_RETIRED, OW_NOT_IMPLEMENTED };
struct ow_owner { int32_t *block[3]; size_t count; struct v4_allocator allocator; };
enum ow_status ow_construct(struct ow_owner *, struct v4_allocator, const size_t[3]);
enum ow_status ow_destroy(struct ow_owner *);
typedef void (*ow_cleanup_fn)(void *);
enum ow_when { OW_ALWAYS, OW_ON_ERROR, OW_ON_SUCCESS };
struct ow_cleanup_entry { ow_cleanup_fn fn; void *context; enum ow_when when; };
struct ow_scope { struct ow_cleanup_entry entries[8]; size_t count; unsigned finishing; };
enum ow_status ow_defer(struct ow_scope *, ow_cleanup_fn, void *, enum ow_when);
enum ow_status ow_finish(struct ow_scope *, int success);
enum ow_status ow_publish(struct ow_owner *published, struct ow_owner *staged);
enum ow_status ow_move(struct ow_owner *destination, struct ow_owner *source);
#if defined(__GNUC__) || defined(__clang__)
#define OW_MUST_USE __attribute__((warn_unused_result))
#else
#define OW_MUST_USE
#endif
OW_MUST_USE enum ow_status ow_consume(struct ow_owner *);
struct ow_loan { size_t offset, length; uint32_t serial; unsigned active, writer, retired; };
struct ow_borrows { struct ow_loan loan[4]; size_t extent; };
struct ow_token { unsigned slot; uint32_t serial; };
enum ow_status ow_borrow(struct ow_borrows *, size_t, size_t, int writer, struct ow_token *);
enum ow_status ow_release(struct ow_borrows *, struct ow_token);
enum ow_status ow_split(struct ow_borrows *, struct ow_token parent, size_t left_length, struct ow_token children[2]);
struct ow_cell { int32_t value; struct ow_borrows borrows; };
enum ow_status ow_cell_read(const struct ow_cell *, struct ow_token, int32_t *);
enum ow_status ow_cell_write(struct ow_cell *, struct ow_token, int32_t);
enum ow_status ow_cell_replace(struct ow_cell *, int32_t, int32_t *old);
struct ow_shared { unsigned strong, weak; ow_cleanup_fn destroy, release; void *context; };
enum ow_status ow_shared_retain(struct ow_shared *);
enum ow_status ow_shared_downgrade(struct ow_shared *);
enum ow_status ow_shared_upgrade(struct ow_shared *);
enum ow_status ow_shared_drop(struct ow_shared *);
enum ow_status ow_shared_weak_drop(struct ow_shared *);
struct ow_atomic_shared { atomic_uint strong, weak; ow_cleanup_fn destroy, release; void *context; };
enum ow_status ow_atomic_retain(struct ow_atomic_shared *);
enum ow_status ow_atomic_downgrade(struct ow_atomic_shared *);
enum ow_status ow_atomic_upgrade(struct ow_atomic_shared *);
enum ow_status ow_atomic_drop(struct ow_atomic_shared *);
enum ow_status ow_atomic_weak_drop(struct ow_atomic_shared *);
struct ow_cow { unsigned refs; int32_t *data; size_t length; struct v4_allocator allocator; };
enum ow_status ow_cow_clone(struct ow_cow *);
enum ow_status ow_cow_write(struct ow_cow **owner, struct ow_cow *fresh, size_t index, int32_t value);
enum ow_status ow_cow_drop(struct ow_cow *);
struct ow_pin { const void *address; uint64_t thread; unsigned pending, cancel; };
enum ow_status ow_pin_init(struct ow_pin *, const void *, uint64_t);
enum ow_status ow_pin_check(const struct ow_pin *, const void *, uint64_t);
enum ow_status ow_pin_submit(struct ow_pin *, const void *, uint64_t);
enum ow_status ow_pin_cancel(struct ow_pin *, uint64_t);
enum ow_status ow_pin_complete(struct ow_pin *, uint64_t);
enum ow_status ow_pin_rebind(struct ow_pin *, const void *, uint64_t);
struct ow_allocator {
 int32_t storage[8][16]; size_t used[8], attempts, fail_at, live, peak, releases;
 unsigned invalid_releases, exhausted_counters;
};
struct v4_allocator ow_allocator(struct ow_allocator *);
/* Two synthetic tenants share one actual int32_t[16] object. */
struct ow_tenant_arena { int32_t data[16]; };
enum ow_status ow_checked_write(struct ow_tenant_arena *, unsigned tenant, unsigned offset,
                          unsigned count, int32_t value);
enum ow_status ow_flawed_write(struct ow_tenant_arena *, unsigned tenant, unsigned offset,
                         unsigned count, int32_t value);
#endif
