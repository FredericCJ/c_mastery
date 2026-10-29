/* Learner work: implement the declared contracts. */
#include "memory.h"

enum mw_status mw_fill(unsigned char *p, size_t backing, struct mw_span s,
                       ptrdiff_t index, size_t n, unsigned char value)
{
    (void)p;
    (void)backing;
    (void)s;
    (void)index;
    (void)n;
    (void)value;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_scan(const unsigned char *p, size_t backing, struct mw_span s,
                       size_t *out)
{
    (void)p;
    (void)backing;
    (void)s;
    (void)out;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_string(unsigned char *p, size_t backing, struct mw_span dst,
                         struct mw_span src, size_t n, size_t *out)
{
    (void)p;
    (void)backing;
    (void)dst;
    (void)src;
    (void)n;
    (void)out;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_move(unsigned char *p, size_t backing, struct mw_span dst,
                       struct mw_span src, size_t n, size_t *out)
{
    (void)p;
    (void)backing;
    (void)dst;
    (void)src;
    (void)n;
    (void)out;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_size(size_t count, size_t stride, size_t header,
                       size_t terminator, size_t alignment, size_t limit,
                       size_t *out)
{
    (void)count;
    (void)stride;
    (void)header;
    (void)terminator;
    (void)alignment;
    (void)limit;
    (void)out;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_signed_size(intmax_t count, size_t stride, size_t header,
                              size_t *out)
{
    (void)count;
    (void)stride;
    (void)header;
    (void)out;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_layout(size_t region, size_t alignment, size_t data,
                         size_t bss, size_t retained, size_t heap, size_t guard,
                         size_t stack, struct mw_layout *out)
{
    (void)region;
    (void)alignment;
    (void)data;
    (void)bss;
    (void)retained;
    (void)heap;
    (void)guard;
    (void)stack;
    (void)out;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_startup(unsigned char *dst, size_t n,
                          const unsigned char *load, size_t ln,
                          struct mw_interval data, struct mw_interval bss,
                          struct mw_interval retained)
{
    (void)dst;
    (void)n;
    (void)load;
    (void)ln;
    (void)data;
    (void)bss;
    (void)retained;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_owner(struct mw_owner *o, struct mw_allocator a, size_t n)
{
    (void)o;
    (void)a;
    (void)n;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_resize(struct mw_owner *o, size_t n, unsigned copying)
{
    (void)o;
    (void)n;
    (void)copying;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_read(const struct mw_owner *o, struct mw_borrow b,
                       unsigned char *out)
{
    (void)o;
    (void)b;
    (void)out;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_drop(struct mw_owner *o, uint32_t generation)
{
    (void)o;
    (void)generation;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_record(struct mw_record *out, struct mw_allocator a,
                         const unsigned char *src, size_t n, unsigned mode,
                         unsigned version, unsigned optional, unsigned fail)
{
    (void)out;
    (void)a;
    (void)src;
    (void)n;
    (void)mode;
    (void)version;
    (void)optional;
    (void)fail;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_serialize(const struct mw_record *r, unsigned char *out,
                            size_t capacity, size_t *length)
{
    (void)r;
    (void)out;
    (void)capacity;
    (void)length;
    return MW_UNIMPLEMENTED;
}

void mw_record_drop(struct mw_record *r)
{
    (void)r;
}

enum mw_status mw_tree(unsigned depth, unsigned breadth, unsigned *work,
                       size_t capacity, unsigned *out, size_t *peak)
{
    (void)depth;
    (void)breadth;
    (void)work;
    (void)capacity;
    (void)out;
    (void)peak;
    return MW_UNIMPLEMENTED;
}

void mw_async_init(struct mw_async *a)
{
    (void)a;
}

enum mw_status mw_submit(struct mw_async *a, uint32_t request,
                         struct mw_ticket *out)
{
    (void)a;
    (void)request;
    (void)out;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_submit_return(struct mw_async *a, struct mw_ticket t)
{
    (void)a;
    (void)t;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_cancel(struct mw_async *a, struct mw_ticket t)
{
    (void)a;
    (void)t;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_terminal(struct mw_async *a, struct mw_ticket t)
{
    (void)a;
    (void)t;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_callback_begin(struct mw_async *a, struct mw_ticket t,
                                 unsigned scratch)
{
    (void)a;
    (void)t;
    (void)scratch;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_callback_end(struct mw_async *a, struct mw_ticket t)
{
    (void)a;
    (void)t;
    return MW_UNIMPLEMENTED;
}

enum mw_status mw_reclaim(struct mw_async *a, struct mw_ticket t)
{
    (void)a;
    (void)t;
    return MW_UNIMPLEMENTED;
}
