#include "resize.h"

enum lr_status lr_init(struct lr_buffer *b, struct lr_allocator a)
{
    (void)b;
    (void)a;
    return LR_INVALID;
}

enum lr_status lr_resize(struct lr_buffer *b, size_t n)
{
    (void)b;
    (void)n;
    return LR_INVALID;
}

void lr_release(struct lr_buffer *b)
{
    (void)b;
}
