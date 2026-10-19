#ifndef CMASTERY_FOUNDATION_DEPTH_H
#define CMASTERY_FOUNDATION_DEPTH_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/* Target assumptions are checked, not silently inferred from hosted tests. */
_Static_assert(CHAR_BIT == 8, "foundation_depth wire format requires octets");
typedef enum { FD_OK, FD_INVALID, FD_RANGE, FD_STATE, FD_NOMEM,
               FD_INIT_FAILED, FD_STALE, FD_FULL, FD_DENIED,
               FD_NOT_IMPLEMENTED } FDStatus;
typedef enum { FD_DEFINED, FD_UNSPECIFIED, FD_IMPLEMENTATION_DEFINED,
               FD_UNDEFINED, FD_CONSTRAINT, FD_UNKNOWN } FDClass;
typedef enum { FD_MIXED_SIGN, FD_UNSIGNED_WRAP, FD_UNSEQUENCED,
               FD_CALL_ORDER, FD_NEGATIVE_SHIFT, FD_SIGNED_OVERFLOW,
               FD_SHIFT_WIDTH, FD_POINTER_CONSTRAINT, FD_ENDED_LIFETIME,
               FD_SHORT_CIRCUIT, FD_CASE_COUNT } FDCase;
FDClass fd_verdict(FDCase question);
FDStatus fd_repair_step(int *value, int *previous);
FDStatus fd_extent(size_t offset, size_t count, size_t width, size_t limit,
                   size_t *out);
FDStatus fd_from_signed(intmax_t value, size_t *out);

typedef struct { const unsigned char *base; size_t length, position; } FDCursor;
FDStatus fd_take(FDCursor *cursor, size_t count, const unsigned char **out);
FDStatus fd_subview(const FDCursor *parent, size_t offset, size_t count,
                    FDCursor *out);
typedef struct { unsigned char flags; uint16_t count; int32_t value; } FDRecord;
FDStatus fd_encode(FDRecord record, unsigned char *out, size_t capacity);
FDStatus fd_decode(const unsigned char *wire, size_t length, FDRecord *out);

typedef struct { int32_t value; uint32_t serial; } FDValue;
typedef struct {
    void *context;
    void *(*allocate)(void *, size_t, size_t);
    void (*deallocate)(void *, void *);
} FDAllocator;
typedef int (*FDInitialize)(void *, FDValue *);
typedef void (*FDDestroy)(void *, FDValue *);
typedef enum { FD_EMPTY, FD_ACQUIRED, FD_INITIALIZED, FD_PUBLISHED } FDSlotState;
typedef struct {
    FDValue *storage;
    FDSlotState state;
    FDAllocator allocator;
    FDDestroy destroy;
    void *callback_context;
} FDSlot;
#define FD_SLOT_INITIALIZER {0, FD_EMPTY, {0, 0, 0}, 0, 0}
FDStatus fd_slot_acquire(FDSlot *slot, FDAllocator allocator);
FDStatus fd_slot_initialize(FDSlot *slot, FDInitialize initialize,
                            FDDestroy destroy, void *context);
FDStatus fd_slot_publish(FDSlot *slot, const FDValue **out);
void fd_slot_reset(FDSlot *slot);

/* Disjoint live arrays are a precondition; it is not tested with pointer order. */
void fd_transform(unsigned char *restrict destination,
                  const unsigned char *restrict source, size_t count,
                  unsigned char mask);
/* All windows are offsets within this one real array. */
FDStatus fd_move(unsigned char *base, size_t length, size_t destination,
                 size_t source, size_t count);
#define FD_POOL_CAPACITY 3u
#define FD_GENERATION_MAX 3u
typedef struct FDPool FDPool;
typedef struct { const FDPool *owner; size_t index; unsigned generation; } FDHandle;
typedef struct { FDValue value; unsigned generation, live, retired; } FDEntry;
struct FDPool { FDEntry entries[FD_POOL_CAPACITY]; };
void fd_pool_init(FDPool *pool);
FDStatus fd_pool_acquire(FDPool *pool, FDValue value, FDHandle *out);
FDStatus fd_pool_get(const FDPool *pool, FDHandle handle, FDValue *out);
FDStatus fd_pool_release(FDPool *pool, FDHandle handle);
FDStatus fd_apply(const unsigned char *wire, size_t length, int authorized,
                  int32_t *synthetic_counter);
#endif
