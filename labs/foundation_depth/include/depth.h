#ifndef CMASTERY_FOUNDATION_DEPTH_H
#define CMASTERY_FOUNDATION_DEPTH_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/* Target assumptions are checked, not silently inferred from hosted tests. */
_Static_assert(CHAR_BIT == 8, "foundation_depth wire format requires octets");

/* Wire record geometry, in octets (CONTRACTS.md, "Canonical eight-octet
 * record"). The header is version, flags and a 16-bit count; the payload is a
 * 32-bit signed value. */
#define FD_WIRE_VERSION 1u
#define FD_WIRE_HEADER_BYTES 4u
#define FD_WIRE_PAYLOAD_BYTES 4u
#define FD_WIRE_BYTES (FD_WIRE_HEADER_BYTES + FD_WIRE_PAYLOAD_BYTES)
/* Only these flag bits may be set in a record. */
#define FD_FLAGS_MASK 3u

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

/* Returns the C17 classification of the supplied case, or FD_UNKNOWN when
 * `question` is not a valid enum fd_case value. Pure. */
enum fd_class fd_verdict(enum fd_case question);

/* Saves *value into *previous, then increments *value once. Both pointers must
 * be non-NULL and distinct (FD_INVALID otherwise). Returns FD_RANGE when
 * *value is INT_MAX. Neither object is modified on any failure. */
enum fd_status fd_repair_step(int *value, int *previous);

/* Computes offset + count * width without wrapping, all in size_t units, and
 * stores it in *out only when it is <= limit (FD_OK). Returns FD_RANGE when
 * the exact value exceeds limit, FD_INVALID when out is NULL. *out is
 * unchanged on failure. */
enum fd_status fd_extent(size_t offset, size_t count, size_t width,
                         size_t limit, size_t *out);

/* Converts a signed value to size_t. Returns FD_RANGE for negative values and
 * for values size_t cannot hold, FD_INVALID when out is NULL; *out is
 * unchanged on failure. */
enum fd_status fd_from_signed(intmax_t value, size_t *out);

/* A borrowed view of a byte array the cursor does not own. Invariant:
 * position <= length, and base is non-NULL whenever length is positive. Both
 * fields are in bytes. */
struct fd_cursor
{
    const unsigned char *base;
    size_t length;
    size_t position;
};

/* Advances the cursor by `count` bytes and stores the address of the first
 * byte consumed in *out. Returns FD_INVALID for a NULL out or a cursor that
 * violates its invariant, FD_RANGE when fewer than `count` bytes remain. The
 * cursor and *out are unchanged on failure. The result borrows from
 * cursor->base. */
enum fd_status fd_take(struct fd_cursor *cursor, size_t count,
                       const unsigned char **out);

/* Creates in *out a position-zero cursor over `count` bytes starting `offset`
 * bytes into the parent's unconsumed suffix. The parent is not advanced and
 * `out` may alias `parent`. Returns FD_INVALID for NULL arguments or an
 * invalid parent, FD_RANGE when the window does not fit. *out is unchanged on
 * failure. */
enum fd_status fd_subview(const struct fd_cursor *parent, size_t offset,
                          size_t count, struct fd_cursor *out);

struct fd_record
{
    unsigned char flags;
    uint16_t count;
    int32_t value;
};

/* Writes the FD_WIRE_BYTES-octet encoding of `record` to `out`. `capacity` is
 * in octets and must equal FD_WIRE_BYTES (FD_RANGE otherwise). Returns
 * FD_INVALID for a NULL out or flag bits outside FD_FLAGS_MASK. `out` is
 * unchanged on failure. */
enum fd_status fd_encode(struct fd_record record, unsigned char *out,
                         size_t capacity);

/* Decodes exactly FD_WIRE_BYTES octets from `wire` into *out. `length` is in
 * octets. Returns FD_RANGE for any other length and FD_INVALID for NULL
 * arguments, a wrong version octet or reserved flag bits. *out is unchanged
 * on failure and must not overlap the input. */
enum fd_status fd_decode(const unsigned char *wire, size_t length,
                         struct fd_record *out);

struct fd_value
{
    int32_t value;
    uint32_t serial;
};

/* Caller-supplied storage provider. allocate(context, size_bytes, alignment)
 * returns suitably aligned, genuinely allocated storage or NULL;
 * deallocate(context, pointer) releases storage obtained from allocate. */
struct fd_allocator
{
    void *context;
    void *(*allocate)(void *, size_t, size_t);
    void (*deallocate)(void *, void *);
};

/* An initializer returns zero on success and undoes its own partial work on
 * failure. A destructor runs once for a successfully initialized value. */
typedef int (*fd_initialize_fn)(void *, struct fd_value *);
typedef void (*fd_destroy_fn)(void *, struct fd_value *);

enum fd_slot_state
{
    FD_EMPTY,
    FD_ACQUIRED,
    FD_INITIALIZED,
    FD_PUBLISHED
};

/* One allocator-provided struct fd_value moving empty -> acquired ->
 * initialized -> published. The slot owns the storage from a successful
 * acquire until fd_slot_reset. Callbacks must not reenter the same slot. */
struct fd_slot
{
    struct fd_value *storage;
    enum fd_slot_state state;
    struct fd_allocator allocator;
    fd_destroy_fn destroy;
    void *callback_context;
};

#define FD_SLOT_INITIALIZER {0, FD_EMPTY, {0, 0, 0}, 0, 0}

/* Requests storage for one struct fd_value through `allocator`. The slot must
 * be empty (FD_STATE otherwise). Returns FD_INVALID for a NULL slot or missing
 * callbacks and FD_NOMEM when allocation fails, leaving the slot empty. */
enum fd_status fd_slot_acquire(struct fd_slot *slot,
                               struct fd_allocator allocator);

/* Runs `initialize(context, storage)` on an acquired slot. On success the slot
 * is initialized and remembers `destroy` and `context`. If the callback
 * fails, the storage is released, the slot is empty again and FD_INIT_FAILED is
 * returned without calling `destroy`. Returns FD_STATE if the slot is not
 * acquired and FD_INVALID for a NULL slot or initializer. */
enum fd_status fd_slot_initialize(struct fd_slot *slot,
                                  fd_initialize_fn initialize,
                                  fd_destroy_fn destroy, void *context);

/* Stores a borrowed pointer to the initialized value in *out. Valid in the
 * initialized and published states (FD_STATE otherwise); *out is unchanged on
 * failure. The pointer expires at fd_slot_reset. */
enum fd_status fd_slot_publish(struct fd_slot *slot,
                               const struct fd_value **out);

/* Destroys (if initialized) and frees whatever the slot holds, returning it to
 * the empty state. A NULL or already empty slot is ignored. */
void fd_slot_reset(struct fd_slot *slot);

/* Writes destination[i] = source[i] ^ mask for i < count. When count is
 * positive the arrays must be live and disjoint (a precondition, not tested
 * with pointer order); with count zero the pointers may be NULL. */
void fd_transform(unsigned char *restrict destination,
                  const unsigned char *restrict source, size_t count,
                  unsigned char mask);

/* Copies `count` bytes from offset `source` to offset `destination` of one
 * array of `length` bytes as if through a snapshot, so windows may overlap.
 * Returns FD_INVALID for a NULL base with positive length and FD_RANGE when
 * either window leaves the array; the array is unchanged on failure. */
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
    unsigned generation;
    unsigned live;
    unsigned retired;
};

struct fd_pool
{
    struct fd_entry entries[FD_POOL_CAPACITY];
};

/* Prepares an empty pool. `pool` must be non-NULL; call once before use and
 * keep the pool at a stable address while handles exist. */
void fd_pool_init(struct fd_pool *pool);

/* Stores `value` in the lowest free, non-retired slot and writes its handle to
 * *out. Returns FD_FULL (leaving *out unchanged) when no slot is available and
 * FD_INVALID for NULL arguments. */
enum fd_status fd_pool_acquire(struct fd_pool *pool, struct fd_value value,
                               struct fd_handle *out);

/* Copies the value named by `handle` to *out. Returns FD_STALE for a foreign,
 * out-of-range, released or superseded handle and FD_INVALID for a NULL pool
 * or out; *out is unchanged on failure. The pool must be live. */
enum fd_status fd_pool_get(const struct fd_pool *pool, struct fd_handle handle,
                           struct fd_value *out);

/* Releases the slot named by `handle`, making that handle stale. Returns
 * FD_STALE for an invalid handle (including a repeated release) and
 * FD_INVALID for a NULL pool. */
enum fd_status fd_pool_release(struct fd_pool *pool, struct fd_handle handle);

/* Decodes one record from `wire`, adds count * value to *synthetic_counter and
 * publishes the sum only when `authorized` is nonzero (trusted by the caller,
 * never derived from wire bytes). Failures take precedence in this order: a
 * NULL counter (FD_INVALID) is rejected first, then decoder errors, then
 * FD_RANGE when the sum does not fit int32_t, then FD_DENIED.
 * *synthetic_counter is unchanged on every failure. */
enum fd_status fd_apply(const unsigned char *wire, size_t length,
                        int authorized, int32_t *synthetic_counter);
#endif
