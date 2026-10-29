#ifndef MEMORY_FOLLOWTHROUGH_H
#define MEMORY_FOLLOWTHROUGH_H
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>

enum mw_status
{
    MW_OK,
    MW_INPUT,
    MW_EXTENT,
    MW_INIT,
    MW_NUL,
    MW_OVERLAP,
    MW_MULTIPLY,
    MW_ADD,
    MW_ZERO,
    MW_ALIGN,
    MW_NOMEM,
    MW_BUSY,
    MW_STALE,
    MW_RETIRED,
    MW_UNIMPLEMENTED = 99
};

/* Offsets describe one actual live backing array, so overlap reasoning never
 * orders unrelated pointers. capacity is the independent logical extent. */
struct mw_span
{
    size_t offset, capacity, initialized;
};
enum mw_status mw_fill(unsigned char *, size_t, struct mw_span, ptrdiff_t,
                       size_t, unsigned char);
enum mw_status mw_scan(const unsigned char *, size_t, struct mw_span, size_t *);
enum mw_status mw_string(unsigned char *, size_t, struct mw_span,
                         struct mw_span, size_t, size_t *);
enum mw_status mw_move(unsigned char *, size_t, struct mw_span, struct mw_span,
                       size_t, size_t *);
enum mw_status mw_size(size_t, size_t, size_t, size_t, size_t, size_t,
                       size_t *);
enum mw_status mw_signed_size(intmax_t, size_t, size_t, size_t *);

struct mw_interval
{
    size_t low, high;
};

struct mw_layout
{
    struct mw_interval data, bss, retained, heap, guard, stack;
    size_t fixed, gap, flash_bytes;
    unsigned fixed_fits, reservation_fits;
};
/* Report representable geometry even when it exceeds region capacity. Invalid
 * arithmetic/alignment leaves the report unchanged. A false fits flag denies use. */
enum mw_status mw_layout(size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                         size_t, struct mw_layout *);
enum mw_status mw_startup(unsigned char *, size_t, const unsigned char *,
                          size_t, struct mw_interval, struct mw_interval,
                          struct mw_interval);

struct mw_allocator
{
    void *context;
    void *(*take)(void *, size_t);
    void *(*resize)(void *, void *, size_t);
    void (*drop)(void *, void *);
};

struct mw_owner
{
    unsigned char *data;
    size_t size;
    uint32_t generation;
    unsigned pins, retired;
    struct mw_allocator allocator;
};

struct mw_borrow
{
    uint32_t generation;
    size_t offset;
};
enum mw_status mw_owner(struct mw_owner *, struct mw_allocator, size_t);
enum mw_status mw_resize(struct mw_owner *, size_t, unsigned);
enum mw_status mw_read(const struct mw_owner *, struct mw_borrow,
                       unsigned char *);
enum mw_status mw_drop(struct mw_owner *, uint32_t);

struct mw_record
{
    uint32_t length, mode, tag, version, optional;
    unsigned char *payload;
    struct mw_allocator allocator;
};
enum mw_status mw_record(struct mw_record *, struct mw_allocator,
                         const unsigned char *, size_t, unsigned, unsigned,
                         unsigned, unsigned);
enum mw_status mw_serialize(const struct mw_record *, unsigned char *, size_t,
                            size_t *);
void mw_record_drop(struct mw_record *);

struct mw_operation
{
    struct f_async operation;
    uint32_t generation, request;
    unsigned started, submit_active, callback_active, callback_pending, retired,
        scratch;
};

struct mw_async
{
    struct mw_operation slots[2];
    unsigned scratch_mask, callbacks, peak_callbacks, results;
};

struct mw_ticket
{
    unsigned slot;
    uint32_t generation, request;
};

void mw_async_init(struct mw_async *);
enum mw_status mw_submit(struct mw_async *, uint32_t, struct mw_ticket *);
enum mw_status mw_submit_return(struct mw_async *, struct mw_ticket);
enum mw_status mw_cancel(struct mw_async *, struct mw_ticket);
enum mw_status mw_terminal(struct mw_async *, struct mw_ticket);
enum mw_status mw_callback_begin(struct mw_async *, struct mw_ticket, unsigned);
enum mw_status mw_callback_end(struct mw_async *, struct mw_ticket);
enum mw_status mw_reclaim(struct mw_async *, struct mw_ticket);
/* Full binary teaching tree: depth<=6, leaves carry value1. Caller work storage
 * owns capacity; on refusal no result is published. breadth=0 DFS,1 BFS. */
enum mw_status mw_tree(unsigned, unsigned, unsigned *, size_t, unsigned *,
                       size_t *);
#endif
