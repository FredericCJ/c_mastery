#include "resize.h"
#ifndef LR_MUTANT
#define LR_MUTANT 0
#endif
enum
{
    BUFFER_LIMIT_BYTES = 64
};

/* Start an empty buffer that borrows allocator callbacks. Returns LR_INVALID
   if b is null or a callback is missing. */
enum lr_status lr_init(struct lr_buffer *b, struct lr_allocator a)
{
    if (!b || !a.resize || !a.release)
    {
        return LR_INVALID;
    }
    b->data = NULL;
    b->size = 0;
    b->allocator = a;
    return LR_OK;
}

/* Resize to 1..64 bytes; new bytes are zeroed. Zero is refused rather than
   forwarded, because what a zero-size request does depends on the allocator
   (free, or return a unique pointer). On LR_INVALID and LR_NOMEM the buffer
   still holds its old block and size. */
enum lr_status lr_resize(struct lr_buffer *b, size_t size)
{
    if (!b || !b->allocator.resize || !b->allocator.release)
    {
        return LR_INVALID;
    }
#if LR_MUTANT == 1
    if (!size)
    {
        (void)b->allocator.resize(b->allocator.context, b->data, b->size, 0);
    }
#endif
    if (!size || size > BUFFER_LIMIT_BYTES)
    {
        return LR_INVALID;
    }
    unsigned char *next =
        b->allocator.resize(b->allocator.context, b->data, b->size, size);
#if LR_MUTANT == 2
    b->data = next;
#endif
    if (!next)
    {
        return LR_NOMEM;
    }
#if LR_MUTANT != 4
    for (size_t i = b->size; i < size; i++)
    {
        next[i] = 0;
    }
#endif
    b->data = next;
    b->size = size;
    return LR_OK;
}

/* Release the block. The buffer is emptied before the callback runs, so the
   callback observes no owner; a repeated release is a no-op. */
void lr_release(struct lr_buffer *b)
{
    if (!b || !b->data)
    {
        return;
    }
    unsigned char *old = b->data;
    size_t size = b->size;
    struct lr_allocator a = b->allocator;
    b->data = NULL;
    b->size = 0;
#if LR_MUTANT == 3
    (void)old;
    (void)size;
    (void)a;
#else
    a.release(a.context, old, size);
#endif
}
