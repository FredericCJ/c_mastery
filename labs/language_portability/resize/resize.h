#ifndef LP_RESIZE_H
#define LP_RESIZE_H
#include <stddef.h>

enum lr_status
{
    LR_OK,
    LR_INVALID,
    LR_NOMEM
};

struct lr_allocator
{
    void *context;
    /* Nonzero operation: preserve old allocation on NULL; success transfers
       min(old,new) bytes to returned owned storage and invalidates old borrows. */
    void *(*resize)(void *, void *, size_t, size_t);
    void (*release)(void *, void *, size_t);
};

struct lr_buffer
{
    unsigned char *data;
    size_t size;
    struct lr_allocator allocator;
};
/* Init only fresh/nonowning control; live noncopied controls are serialized.
   Callbacks are nonreentrant, do not mutate control, and retain no borrowed
   pointer. Resize admits1..64 only; zero is refused before any callback. */
enum lr_status lr_init(struct lr_buffer *, struct lr_allocator);
enum lr_status lr_resize(struct lr_buffer *, size_t);
void lr_release(struct lr_buffer *);
#endif
