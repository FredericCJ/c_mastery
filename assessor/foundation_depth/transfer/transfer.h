#ifndef FD_TRANSFER_H
#define FD_TRANSFER_H
#include "depth.h"

/* Withheld changed-condition interfaces. Release only with the transfer
 * brief. */
struct fd_step
{
    enum fd_status status;
    int old_value, new_value;
};
/* Returns old_value = value and new_value = value + 1 with FD_OK, or FD_RANGE
 * with new_value = value when value is INT_MAX. Never overflows. */
struct fd_step fd_step_value(int value);

struct fd_layout
{
    size_t first;
    size_t second;
    size_t end;
};
/* Lays out a header, then an array of first_count elements of first_width
 * bytes, then an array of second_count elements of second_width bytes, all
 * within `limit` bytes. Negative counts and any extent beyond limit return
 * FD_RANGE; *out is written only on FD_OK. */
enum fd_status fd_layout(size_t header, intmax_t first_count,
                         size_t first_width, intmax_t second_count,
                         size_t second_width, size_t limit,
                         struct fd_layout *out);

struct fd_segment
{
    const unsigned char *base;
    size_t length;
};

struct fd_segmented
{
    struct fd_segment segments[2];
    size_t position;
};

struct fd_owner_view
{
    const unsigned char *owner;
    size_t owner_length;
    size_t begin;
    size_t end;
    size_t position;
};
/* Like fd_subview for a view that remembers its owner and window. Returns
 * FD_INVALID for NULL arguments or an inconsistent source, FD_RANGE when the
 * window does not fit; *out may alias source and is unchanged on failure. */
enum fd_status fd_owner_subview(const struct fd_owner_view *source,
                                size_t offset, size_t count,
                                struct fd_owner_view *out);
/* Reads a two-octet tag then a two-octet little-endian value as one unit.
 * The cursor advances and *out is written only on FD_OK; FD_INVALID for a bad
 * tag or NULL arguments, FD_RANGE when too few bytes remain. */
enum fd_status fd_read_tagged(struct fd_cursor *source, uint16_t *out);
/* Copies `count` bytes from the two segments, read as one logical array
 * starting at cursor->position, into `destination` (capacity bytes). The
 * position advances only on FD_OK; FD_RANGE when count exceeds the remaining
 * bytes or the capacity, FD_INVALID for an inconsistent cursor. */
enum fd_status fd_copy_segments(struct fd_segmented *cursor, size_t count,
                                unsigned char *destination, size_t capacity);
/* Version-2 record: eight octets, little-endian, count <= UINT16_MAX and
 * value within int32_t. capacity must equal FD_WIRE_BYTES. FD_RANGE otherwise
 * with `out` unchanged; FD_INVALID for a NULL out. */
enum fd_status fd_encode_v2(uintmax_t count, intmax_t value, unsigned char *out,
                            size_t capacity);
/* Inverse of fd_encode_v2. FD_RANGE for any length other than FD_WIRE_BYTES,
 * FD_INVALID for NULL arguments or a bad version or reserved octet. Outputs
 * are unchanged on failure. */
enum fd_status fd_decode_v2(const unsigned char *wire, size_t length,
                            uintmax_t *count, intmax_t *value);

struct fd_least_record
{
    uint_least16_t count;
    int_least32_t value;
};
/* Least-width record variants of the version-2 codec; same errors. */
enum fd_status fd_least_encode(struct fd_least_record record,
                               unsigned char *out, size_t capacity);
enum fd_status fd_least_decode(const unsigned char *wire, size_t length,
                               struct fd_least_record *out);

/* The array adapter owns an actual declared FDValue; it never treats bytes
 * as one. */
struct fd_typed_storage
{
    struct fd_value value;
    unsigned reserved;
};

/* Allocator adapter over one caller-owned struct fd_typed_storage (the
 * context). Hands out its single value once, and only for the exact size and
 * alignment of struct fd_value; otherwise returns NULL. */
void *fd_typed_allocate(void *context, size_t size, size_t alignment);
void fd_typed_release(void *context, void *value);
/* Moves ownership of a non-empty slot into an empty destination; the source
 * becomes empty. FD_STATE otherwise, FD_INVALID for NULL or identical slots. */
enum fd_status fd_slot_transfer(struct fd_slot *destination,
                                struct fd_slot *source);
/* fd_move over logical_length elements spaced `stride` bytes apart inside
 * physical_length bytes. FD_RANGE when the elements do not fit the physical
 * array or a window leaves the logical array; FD_INVALID for a zero stride. */
enum fd_status fd_stride_move(unsigned char *base, size_t physical_length,
                              size_t logical_length, size_t stride,
                              size_t destination, size_t source, size_t count);

struct fd_stream_view
{
    unsigned char *base;
    size_t length;
    size_t position;
    size_t epoch;
};
/* Moves the unread bytes to the front of the stream, sets length to the
 * unread count, resets position to zero and advances epoch. FD_FULL (nothing
 * moved) when epoch is SIZE_MAX. */
enum fd_status fd_compact(struct fd_stream_view *stream);
/* XORs `length` bytes of one array with `mask` in place. A NULL base is
 * admitted only with length zero (FD_INVALID otherwise). */
enum fd_status fd_inplace_xor(unsigned char *base, size_t length,
                              unsigned char mask);
/* Stores a pointer into the pool's own entry for a valid handle, without
 * copying. Errors as fd_pool_get; *out is unchanged on failure. The pointer
 * expires when the handle is released or the pool is moved. */
enum fd_status fd_pool_borrow(const struct fd_pool *pool,
                              struct fd_handle handle,
                              const struct fd_value **out);

/* The registry itself has a stable lifetime. Mappings may move between live
 * owners. */
#define FD_REGISTRY_SLOTS 2u
/* Epochs run 1 through FD_EPOCH_MAX; the slot retires after the last unbind. */
#define FD_EPOCH_MAX 3u

struct fd_registry_entry
{
    struct fd_pool *owner;
    unsigned epoch;
    unsigned retired;
};

struct fd_registry
{
    struct fd_registry_entry entries[FD_REGISTRY_SLOTS];
};

struct fd_registry_handle
{
    size_t registry_slot;
    size_t local_slot;
    unsigned epoch;
    unsigned generation;
};

/* Empties every slot of the registry. The registry does not own its pools:
 * each bound pool must stay live and at a stable address until it is unbound
 * or relocated, and the registry itself must stay at a stable address. */
void fd_registry_init(struct fd_registry *registry);
/* Binds a pool to an unbound slot; one binding per pool. FD_FULL for a
 * retired slot, FD_STATE when the slot or pool is already bound. */
enum fd_status fd_registry_bind(struct fd_registry *registry, size_t slot,
                                struct fd_pool *pool);
/* Unbinds a slot and advances its epoch, making older handles stale. The
 * slot retires after epoch FD_EPOCH_MAX. FD_STATE if nothing is bound. */
enum fd_status fd_registry_unbind(struct fd_registry *registry, size_t slot);
/* Moves the slot's pool contents to `destination`, which must not already be
 * bound; handles stay valid and the old pool is retired. */
enum fd_status fd_registry_relocate(struct fd_registry *registry, size_t slot,
                                    struct fd_pool *destination);
/* fd_pool_acquire through the slot's bound pool; the handle also carries the
 * slot's epoch. FD_STATE if the slot is unbound. */
enum fd_status fd_registry_acquire(struct fd_registry *registry, size_t slot,
                                   struct fd_value value,
                                   struct fd_registry_handle *out);
/* Resolves a registry handle. FD_STALE for a bad slot, an unbound or retired
 * slot, an older epoch or a stale pool handle; *out unchanged on failure. */
enum fd_status fd_registry_get(const struct fd_registry *registry,
                               struct fd_registry_handle handle,
                               struct fd_value *out);

struct fd_frame
{
    unsigned char bytes[FD_WIRE_BYTES];
    size_t used;
    int authority;
};

enum fd_effect
{
    FD_QUERY,
    FD_UPDATE
};
/* Decodes the record and, for FD_UPDATE, applies it through fd_apply; reports
 * the resulting counter in *observed on FD_OK. Failures leave both outputs
 * unchanged. */
enum fd_status fd_effect(enum fd_effect effect, const unsigned char *wire,
                         size_t length, int authority, int32_t *counter,
                         int32_t *observed);
/* Discards a partly fed record, including its latched authority. */
void fd_frame_cancel(struct fd_frame *frame);
/* Appends bytes to a record under assembly. When the record completes it is
 * applied with the authority latched from its first chunk and the frame is
 * cleared; *committed is 1 when a complete record was applied successfully,
 * even if its delta is zero, and does not mean the counter changed. FD_RANGE
 * (frame unchanged) when the bytes would overflow the record. */
enum fd_status fd_frame_feed(struct fd_frame *frame, const unsigned char *bytes,
                             size_t count, int authority, int32_t *counter,
                             int *committed);
#endif
