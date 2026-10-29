#include "capture.h"

struct d_allocator ds_counted_bytes(struct ow_allocator *a)
{
    (void)a;
    return (struct d_allocator){0};
}

int ds_capture_i32(struct ds_erased *a, const int32_t *b, size_t c,
                   void (*d)(void *), void *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return DS_FAILED;
}

int ds_borrowed_i32(const int32_t *a, size_t b, int c, int *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return DS_FAILED;
}

void ds_typed_init(struct ds_typed_capture *a, struct v4_allocator b)
{
    (void)a;
    (void)b;
}

int ds_typed_assign(struct ds_typed_capture *a, const int32_t *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return DS_FAILED;
}

int ds_typed_call(const struct ds_typed_capture *a, int b, int *c)
{
    (void)a;
    (void)b;
    (void)c;
    return DS_FAILED;
}

int ds_typed_move(struct ds_typed_capture *a, struct ds_typed_capture *b)
{
    (void)a;
    (void)b;
    return DS_FAILED;
}

void ds_typed_drop(struct ds_typed_capture *a)
{
    (void)a;
}
