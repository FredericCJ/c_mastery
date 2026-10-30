/*
 * Effects laboratory: bounded components whose side effects are explicit.
 *
 * Common rules (the full contract is CONTRACTS.md):
 *  - Every object passed in must be live, aligned and initialized. A length
 *    argument of zero admits a null data pointer; any other length does not.
 *  - Output objects and input ranges are distinct unless a function says so.
 *  - A rejected call leaves its outputs and logical state unchanged, except
 *    for the diagnostic counters and the failure latch named below.
 *  - None of these functions waits on a lock or a clock; callbacks they invoke
 *    are the caller's responsibility. Instances need external serialization.
 */
#ifndef CMASTERY_EFFECTS_DEPTH_H
#define CMASTERY_EFFECTS_DEPTH_H
#include <stddef.h>
#include <stdint.h>

/* Capacities, in bytes or slots. */
#define EF_SCRATCH_BYTES 32u
#define EF_POOL_SLOTS 16u
#define EF_RECORD_PAYLOAD_BYTES 32u

enum ef_status
{
    EF_OK,
    EF_INVALID,
    EF_FULL,
    EF_BUSY,
    EF_NOMEM,
    EF_INCOMPLETE,
    EF_FAILED
};

/*
 * Storage supplier. take() returns at least the requested number of bytes or
 * NULL; the storage stays live until release() receives it. Callbacks must not
 * reenter the component that called them.
 */
struct ef_allocator
{
    void *context;
    unsigned char *(*take)(void *context, size_t bytes);
    void (*release)(void *context, unsigned char *storage);
};

/*
 * Checksum cache. Initialize with a valid allocator, generation 1, and
 * scratch, borrows, failed and permanent all zero. generation counts resets;
 * borrows counts outstanding read-only borrows of scratch.
 */
struct ef_cache
{
    struct ef_allocator allocator;
    unsigned char *scratch;
    uint32_t generation;
    uint32_t borrows;
    unsigned failed;
    unsigned permanent;
};

/*
 * Sum of at most EF_SCRATCH_BYTES bytes. Returns EF_OK with *out set;
 * EF_INVALID (outputs unchanged) for a null out, a null data pointer with a
 * nonzero length, or a length above EF_SCRATCH_BYTES.
 */
enum ef_status ef_checksum(const unsigned char *data, size_t length,
                           uint32_t *out);

/*
 * Same input and result as ef_checksum, but copies the bytes into storage that
 * the cache acquires lazily and then retains. Statuses: EF_INVALID (bad
 * argument, zero generation or missing allocator callback), EF_BUSY (a borrow
 * is outstanding), EF_FAILED (permanent failure latched), EF_NOMEM (the
 * allocator returned NULL; the latch is set and nothing is published).
 */
enum ef_status ef_cached(struct ef_cache *cache, const unsigned char *data,
                         size_t length, uint32_t *out);

/*
 * Releases the storage (at most once), clears the failure latch and increments
 * the generation. EF_INVALID for a bad cache, EF_BUSY while a borrow exists,
 * EF_FULL (cache unchanged) when the generation cannot be incremented.
 */
enum ef_status ef_cache_reset(struct ef_cache *cache);

/*
 * Read-only access to the retained storage, valid until the matching
 * ef_cache_return. The caller remembers the admitted length. EF_INVALID when
 * nothing is retained, EF_FULL when the borrow count cannot be incremented.
 */
enum ef_status ef_cache_borrow(struct ef_cache *cache,
                               const unsigned char **out);

/* Ends one borrow. EF_INVALID when no borrow is outstanding. */
enum ef_status ef_cache_return(struct ef_cache *cache);

/*
 * Pool of EF_POOL_SLOTS one-byte slots. Start zeroed. Bit i of used is set
 * while slot i is allocated; requests counts admitted ef_pool_take calls.
 */
struct ef_pool
{
    unsigned char bytes[EF_POOL_SLOTS];
    uint32_t used;
    uint32_t requests;
};

/*
 * Counts the request, then hands out the lowest free slot through *out.
 * EF_FULL when no slot is free (the request still counts) or when the request
 * counter is saturated (nothing changes). EF_INVALID for a null argument.
 */
enum ef_status ef_pool_take(struct ef_pool *pool, unsigned char **out);

/* Frees an allocated slot. EF_INVALID for a bad index or a free slot. */
enum ef_status ef_pool_return(struct ef_pool *pool, unsigned index);

/*
 * Parser with its own scratch. Initialize with cursor 0, the chosen seed,
 * generation 1 and borrowed 0. cursor counts admitted bytes in total.
 */
struct ef_parser
{
    unsigned char scratch[EF_SCRATCH_BYTES];
    size_t cursor;
    uint32_t seed;
    uint32_t generation;
    unsigned borrowed;
};

/* Borrowed view of a parser's scratch; valid until ef_view_release. */
struct ef_view
{
    struct ef_parser *owner;
    uint32_t generation;
    size_t length;
    const unsigned char *data;
};

/*
 * Copies at most EF_SCRATCH_BYTES bytes (ASCII lowercase becomes uppercase),
 * advances the cursor and returns one borrowed view. EF_INVALID for bad
 * arguments, EF_BUSY while a view is outstanding, EF_FULL when the cursor
 * would wrap. *out is written only on EF_OK.
 */
enum ef_status ef_parse(struct ef_parser *parser, const unsigned char *data,
                        size_t length, struct ef_view *out);

/* Ends a view and clears it. EF_INVALID for a stale or already cleared view. */
enum ef_status ef_view_release(struct ef_view *view);

/*
 * Restarts the parser with a new seed: cursor 0, scratch zeroed, generation
 * incremented. EF_BUSY while a view is outstanding, EF_FULL at the maximum
 * generation, EF_INVALID for a bad parser.
 */
enum ef_status ef_parser_reset(struct ef_parser *parser, uint32_t seed);

/*
 * Next value of a 32-bit linear congruential sequence (not cryptographic).
 * The parser must be live; there is no failure return.
 */
uint32_t ef_random(struct ef_parser *parser);

/*
 * Attempt callback: positive means done, zero means retry, negative means
 * failed. It receives a buffer and that buffer's size in bytes.
 */
typedef int (*ef_callback_fn)(void *context, unsigned char *buffer,
                              size_t buffer_bytes);

/*
 * Calls callback at most attempts (0..16) times until it reports done. *used
 * counts completed calls. EF_OK on success, EF_FAILED on a negative result,
 * EF_INCOMPLETE when attempts run out, EF_INVALID for bad arguments or a zero
 * nonblocking flag (the caller asserts the callback never blocks).
 */
enum ef_status ef_attempt(ef_callback_fn callback, void *context,
                          unsigned nonblocking, unsigned attempts,
                          unsigned *used);

struct ef_record
{
    int key;
    unsigned identity;
    unsigned char payload[EF_RECORD_PAYLOAD_BYTES];
};

/*
 * Stable insertion sort of 0..16 elements into nondecreasing order, with
 * constant auxiliary storage. EF_INVALID (array unchanged) for a longer
 * array or a null pointer with a nonzero count.
 */
enum ef_status ef_sort(int *values, size_t count);

/* As ef_sort, ordering whole records by key. Equal keys keep their order. */
enum ef_status ef_sort_records(struct ef_record *records, size_t count);
#endif
