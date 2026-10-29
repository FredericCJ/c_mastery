#ifndef CMASTERY_FOUNDATION_DEPTH_H
#define CMASTERY_FOUNDATION_DEPTH_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/* Target assumptions are checked, not silently inferred from hosted tests. */
_Static_assert(CHAR_BIT == 8, "foundation_depth wire format requires octets");

enum fd_status
{
    FD_OK,
    FD_INVALID,
    FD_RANGE,
    FD_STATE,
    FD_NOMEM,
    FD_INIT_FAILED,
    FD_STALE,
    FD_FULL,
    FD_DENIED,
    FD_NOT_IMPLEMENTED
};

enum fd_class
{
    FD_DEFINED,
    FD_UNSPECIFIED,
    FD_IMPLEMENTATION_DEFINED,
    FD_UNDEFINED,
    FD_CONSTRAINT,
    FD_UNKNOWN
};

enum fd_case
{
    FD_MIXED_SIGN,
    FD_UNSIGNED_WRAP,
    FD_UNSEQUENCED,
    FD_CALL_ORDER,
    FD_NEGATIVE_SHIFT,
    FD_SIGNED_OVERFLOW,
    FD_SHIFT_WIDTH,
    FD_POINTER_CONSTRAINT,
    FD_ENDED_LIFETIME,
    FD_SHORT_CIRCUIT,
    FD_CASE_COUNT
};
enum fd_class fd_verdict(enum fd_case question);
enum fd_status fd_repair_step(int *value, int *previous);
enum fd_status fd_extent(size_t offset, size_t count, size_t width,
                         size_t limit, size_t *out);
enum fd_status fd_from_signed(intmax_t value, size_t *out);

struct fd_cursor
{
    const unsigned char *base;
    size_t length, position;
};
enum fd_status fd_take(struct fd_cursor *cursor, size_t count,
                       const unsigned char **out);
enum fd_status fd_subview(const struct fd_cursor *parent, size_t offset,
                          size_t count, struct fd_cursor *out);

struct fd_record
{
    unsigned char flags;
    uint16_t count;
    int32_t value;
};
enum fd_status fd_encode(struct fd_record record, unsigned char *out,
                         size_t capacity);
enum fd_status fd_decode(const unsigned char *wire, size_t length,
                         struct fd_record *out);

struct fd_value
{
    int32_t value;
    uint32_t serial;
};

struct fd_allocator
{
    void *context;
    void *(*allocate)(void *, size_t, size_t);
    void (*deallocate)(void *, void *);
};

typedef int (*fd_initialize_fn)(void *, struct fd_value *);
typedef void (*fd_destroy_fn)(void *, struct fd_value *);

enum fd_slot_state
{
    FD_EMPTY,
    FD_ACQUIRED,
    FD_INITIALIZED,
    FD_PUBLISHED
};

struct fd_slot
{
    struct fd_value *storage;
    enum fd_slot_state state;
    struct fd_allocator allocator;
    fd_destroy_fn destroy;
    void *callback_context;
};

#define FD_SLOT_INITIALIZER {0, FD_EMPTY, {0, 0, 0}, 0, 0}
enum fd_status fd_slot_acquire(struct fd_slot *slot,
                               struct fd_allocator allocator);
enum fd_status fd_slot_initialize(struct fd_slot *slot,
                                  fd_initialize_fn initialize,
                                  fd_destroy_fn destroy, void *context);
enum fd_status fd_slot_publish(struct fd_slot *slot,
                               const struct fd_value **out);
void fd_slot_reset(struct fd_slot *slot);

/* Disjoint live arrays are a precondition; it is not tested with pointer order. */
void fd_transform(unsigned char *restrict destination,
                  const unsigned char *restrict source, size_t count,
                  unsigned char mask);
/* All windows are offsets within this one real array. */
enum fd_status fd_move(unsigned char *base, size_t length, size_t destination,
                       size_t source, size_t count);
#define FD_POOL_CAPACITY 3u
#define FD_GENERATION_MAX 3u
struct fd_pool;

struct fd_handle
{
    const struct fd_pool *owner;
    size_t index;
    unsigned generation;
};

struct fd_entry
{
    struct fd_value value;
    unsigned generation, live, retired;
};

struct fd_pool
{
    struct fd_entry entries[FD_POOL_CAPACITY];
};

void fd_pool_init(struct fd_pool *pool);
enum fd_status fd_pool_acquire(struct fd_pool *pool, struct fd_value value,
                               struct fd_handle *out);
enum fd_status fd_pool_get(const struct fd_pool *pool, struct fd_handle handle,
                           struct fd_value *out);
enum fd_status fd_pool_release(struct fd_pool *pool, struct fd_handle handle);
enum fd_status fd_apply(const unsigned char *wire, size_t length,
                        int authorized, int32_t *synthetic_counter);
#endif
