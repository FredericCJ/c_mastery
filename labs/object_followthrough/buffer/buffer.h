#ifndef OB_BUFFER_H
#define OB_BUFFER_H
#include <stddef.h>
#include <stdint.h>

/*
 * Contract conventions. Unless a comment says otherwise an int function
 * returns 0 on success and -1 when the request is rejected, and a rejected
 * request leaves every output and control object as it was. Control objects
 * are live and correctly typed (zero-initialized where the comment says
 * "empty") and distinct from input and output pointers. Callbacks supply real
 * allocated storage, are not re-entered and are not called concurrently;
 * nothing here blocks or is thread-safe. CONTRACTS.md is the normative text.
 */

/*
 * Injected allocation callbacks. context is passed back unchanged.
 * resize(context, block, old_bytes, new_bytes) either returns a block holding
 * the first min(old, new) bytes (the old block is then invalid) or returns
 * NULL leaving the old block valid and unchanged.
 */
struct ob_allocator
{
    void *context;
    void *(*allocate)(void *context, size_t bytes);
    void *(*resize)(void *context, void *block, size_t old_bytes,
                    size_t new_bytes);
    void (*release)(void *context, void *block);
};

struct ob_record
{
    int number;
    void *pointer;
    char tag;
};

/*
 * A record under construction. Bit 0 of initialized (value 1) authorizes
 * reading value.number, bit 1 (value 2) value.pointer and bit 2 (value 4)
 * value.tag; no member may be read unless its bit is set.
 */
struct ob_construction
{
    struct ob_record value;
    unsigned initialized;
};

/*
 * Builds *construction field by field, setting the matching mask bit after
 * each store. fail_after == 1 reports failure right after the number is
 * stored, fail_after == 2 right after the pointer is stored; any other value
 * completes all three fields. Returns 0 when complete and -1 on the injected
 * failure or a NULL construction. A successful pointer store transfers
 * ownership of pointer to the construction; before it the caller keeps it.
 */
int ob_construct(struct ob_construction *construction, int number,
                 void *pointer, char tag, unsigned fail_after);

/* Field-wise equality (padding is never compared). 1 equal, 0 otherwise or
 * if either argument is NULL. Only fully initialized records may be passed. */
int ob_record_equal(const struct ob_record *left,
                    const struct ob_record *right);

/*
 * Writes exactly four bytes: number (0..65535) big-endian, the tag byte, and
 * 1 or 0 for pointer present or null. capacity_bytes must be at least 4.
 * A number outside 0..65535, a NULL argument or a short capacity returns -1.
 * Addresses and padding are never serialized; later bytes of out are kept.
 */
int ob_record_encode(const struct ob_record *record, unsigned char *out,
                     size_t capacity_bytes);

/*
 * Releases the pointer only if its mask bit says the pointer store completed
 * (and it is not NULL), then clears the mask. Returns -1 for a NULL
 * construction or an allocator without release.
 */
int ob_cleanup(struct ob_construction *construction,
               struct ob_allocator allocator);

/*
 * Total size in bytes of prefix_bytes + count * width_bytes, computed without
 * overflow and checked against limit_bytes. Rejected for width 0, a prefix
 * above the limit, or a total above the limit.
 */
int ob_size(size_t prefix_bytes, size_t width_bytes, size_t count,
            size_t limit_bytes, size_t *out);

/* One allocation: header immediately followed by capacity payload bytes. */
struct ob_flex
{
    size_t length, capacity; /* payload bytes in use / available */
    unsigned char payload[];
};

/* A header with a double, a char and flexible bytes: its size, the offset
 * of payload and the tail padding are what the laboratory measures. */
struct ob_padded
{
    double alignment;
    unsigned char tag;
    unsigned char payload[];
};

/*
 * Owner of one struct ob_flex. A new table is {0}: value NULL, not retired.
 * identity is externally assigned and nonzero. generation starts at 1 and is
 * advanced by each successful resize or erase; it never exceeds 3, so only
 * two invalidating operations can succeed and later ones are refused.
 * retired is set permanently by reset or by being moved from.
 */
struct ob_table
{
    struct ob_flex *value;
    struct ob_allocator allocation;
    uint32_t identity, generation;
    int retired;
};

/* A logical reference: identity and generation must match the table, and
 * index must still be in range, for it to resolve. */
struct ob_handle
{
    uint32_t identity, generation;
    size_t index;
};

/*
 * Creates an empty table with capacity zeroed payload bytes in a single
 * allocation. Requires a never-used table, a nonzero identity and an
 * allocator with all three callbacks. Generation becomes 1.
 */
int ob_create(struct ob_table *table, struct ob_allocator allocator,
              uint32_t identity, size_t capacity);

/*
 * Changes the payload capacity to a nonzero value, preserving the common
 * prefix, truncating the length on shrink and zeroing new bytes. Consumes one
 * generation. The allocator's resize is not called for capacity 0, for a
 * retired table or when no generation is left. If it fails, pointer, contents,
 * capacity and generation are unchanged. Success invalidates every raw pointer
 * into the old block and every outstanding handle.
 */
int ob_resize(struct ob_table *table, size_t capacity);

/* Appends one byte if capacity remains; never relocates the block. */
int ob_append(struct ob_table *table, unsigned char byte);

/*
 * Removes the byte at index by shifting later bytes down, without allocating.
 * Consumes one generation, so existing handles stop resolving. Refused (-1)
 * for a table that is not live, an index out of range, or once the generation
 * is exhausted.
 */
int ob_erase(struct ob_table *table, size_t index);

/* Releases the block and retires the table (and its identity) permanently.
 * NULL is ignored; a table with no block is only retired. */
void ob_reset(struct ob_table *table);

/* Makes a handle for the byte at index (needs a live, unretired table). */
int ob_handle(const struct ob_table *table, size_t index,
              struct ob_handle *out);

/* Reads the byte a handle names; rejected on identity, generation or range
 * mismatch, or for a table without a live block. */
int ob_resolve(const struct ob_table *table, struct ob_handle handle,
               unsigned char *out);

/*
 * Transfers ownership from a live table to a distinct empty one, keeping
 * identity and generation; the source becomes retired with identity 0.
 */
int ob_table_move(struct ob_table *to, struct ob_table *from);

/* Representation B: header and payload are separate allocations. */
struct ob_split
{
    size_t length, capacity;
    unsigned char *payload;
    struct ob_allocator allocation;
};

/*
 * Allocates a header and, when capacity > 0, a zeroed payload. If the second
 * allocation fails the header is released and *out is not written.
 */
int ob_split_create(struct ob_allocator allocator, size_t capacity,
                    struct ob_split **out);

/* Releases payload then header and sets *owner to NULL. NULL is ignored. */
void ob_split_drop(struct ob_split **owner);

/* Representation C: immutable shared bytes with at most three references. */
struct ob_shared
{
    size_t references, length;
    struct ob_allocator allocation;
    unsigned char payload[];
};

/* Copies n bytes (n is a byte count; bytes may be NULL only when n is 0)
 * once into a new block with one reference. */
int ob_shared_create(struct ob_allocator allocator, const unsigned char *bytes,
                     size_t n, struct ob_shared **out);

/* Adds a reference (*out = shared). Rejected, unchanged, at three. */
int ob_shared_retain(struct ob_shared *shared, struct ob_shared **out);

/* Drops *owner's reference and sets it to NULL; the last drop frees. */
void ob_shared_drop(struct ob_shared **owner);

/*
 * Stores 1..requested in a working array and returns their sum in *result.
 * requested must be in 1..16 and at most capacity (storage elements), and is
 * checked before any array is declared. The working array is a local
 * variable-length array, or the caller's storage when the build defines
 * OB_NO_VLA (or the compiler has no VLAs). *evaluations receives the number of
 * times the array-size expression was evaluated (0 without a VLA). Rejected
 * requests leave result and evaluations unchanged.
 */
int ob_workspace(int requested, int *storage, size_t capacity, int *result,
                 unsigned *evaluations);
#endif
