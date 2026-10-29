#include "dispatch_extensions.h"
#include <limits.h>
#ifndef DS_EXT_MUTANT
#define DS_EXT_MUTANT 0
#endif
static int int_measure(const void *p, int *out)
{
    *out = *(const int *)p;
    return DS_OK;
}

static int bytes_measure(const void *p, int *out)
{
    const struct ds_bytes_receiver *r = p;
    if ((!r->bytes && r->length) || r->length > INT_MAX)
        return DS_INVALID;
#if DS_EXT_MUTANT == 1
    *out = 0;
#else
    *out = (int)r->length;
#endif
    return DS_OK;
}
static const struct ds_ops int_ops = {int_measure}, byte_ops = {bytes_measure};

struct ds_int_interface ds_int_interface(const struct ds_int_receiver *r)
{
    return (struct ds_int_interface){{r ? r->object : NULL, &int_ops}};
}

struct ds_byte_interface ds_byte_interface(const struct ds_bytes_receiver *r)
{
#if DS_EXT_MUTANT == 2
    (void)r;
    return (struct ds_byte_interface){{NULL, &byte_ops}};
#else
    return (struct ds_byte_interface){{r, &byte_ops}};
#endif
}

struct ds_progress ds_read_progress(struct ds_reader r, unsigned char *out,
                                    size_t capacity)
{
    struct ds_progress result = {DS_INVALID, 0};
    if (!r.read || (!out && capacity) || capacity > 16)
        return result;
    unsigned char staged[16];
    size_t count = 0;
    int status = r.read(r.context, staged, capacity, &count);
    if (count > capacity ||
        (status != DS_OK && status != DS_FAILED && status != DS_EMPTY) ||
        (status == DS_EMPTY && count))
        return result;
    result = (struct ds_progress){status, count};
#if DS_EXT_MUTANT == 3
    if (status != DS_OK)
        return (struct ds_progress){status, 0};
#elif DS_EXT_MUTANT == 4
    result.status = DS_OK;
#endif
    for (size_t i = 0; i < count; ++i)
        out[i] = staged[i];
    return result;
}

int ds_borrow_closure_init(struct ds_borrow_closure *c, const int32_t *p,
                           size_t n, unsigned once)
{
    if (!c || (!p && n) || n < 1 || n > 8 || once > 1)
        return DS_INVALID;
    *c = (struct ds_borrow_closure){p, n, once, 1};
    return DS_OK;
}

int ds_borrow_closure_call(struct ds_borrow_closure *c, int arg, int *out)
{
    if (!c || !out)
        return DS_INVALID;
    if (!c->active)
        return DS_EMPTY;
    if (c->once)
        c->active = 0;
    return ds_borrowed_i32(c->values, c->count, arg, out);
}

int ds_owned_closure_init(struct ds_owned_closure *c, struct ds_resource r)
{
    if (!c || !r.object || !r.drop)
        return DS_INVALID;
    *c = (struct ds_owned_closure){r, 1};
    return DS_OK;
}

int ds_owned_closure_call(struct ds_owned_closure *c, ds_owned_body_fn body,
                          int arg, struct ds_owned_result *out)
{
    if (!c || !body || !out || out->retry.object || out->retry.drop)
        return DS_INVALID;
    if (!c->active)
        return DS_EMPTY;
    struct ds_resource resource = c->resource;
#if DS_EXT_MUTANT != 5
    c->resource = (struct ds_resource){0};
    c->active = 0;
#endif
    struct ds_owned_result result = body(resource, arg);
#if DS_EXT_MUTANT == 6
    if (result.retry.object)
    {
        result.retry.drop(result.retry.object);
        result.retry = (struct ds_resource){0};
    }
#endif
    *out = result;
    return DS_OK;
}

void ds_owned_closure_drop(struct ds_owned_closure *c)
{
    if (c->active)
    {
        struct ds_resource r = c->resource;
        c->active = 0;
        c->resource = (struct ds_resource){0};
        r.drop(r.object);
    }
}

static void drop_parts(void *context)
{
    struct ds_clone_capture *c = context;
    for (unsigned i = 2; i;)
    {
        --i;
        if (c->parts[i])
        {
            c->ops.drop(c->ops.context, c->parts[i]);
            c->parts[i] = NULL;
        }
    }
}

void ds_clone_capture_init(struct ds_clone_capture *c, struct d_allocator a,
                           struct ds_clone_ops ops)
{
    ds_erased_init(&c->erased, a);
    c->ops = ops;
    c->parts[0] = c->parts[1] = NULL;
}

int ds_clone_capture_adopt(struct ds_clone_capture *c,
                           const unsigned char *bytes, size_t n,
                           ds_thunk_fn call, void *a, void *b)
{
    if (!c || !a || !b || a == b || !c->ops.clone || !c->ops.drop ||
        c->parts[0] || c->parts[1])
        return DS_INVALID;
    int status = ds_erased_assign(&c->erased, bytes, n, call, drop_parts, c);
    if (status)
        return status;
    c->parts[0] = a;
    c->parts[1] = b;
    return DS_OK;
}

int ds_clone_capture(struct ds_clone_capture *dest,
                     const struct ds_clone_capture *source)
{
    if (!dest || !source || dest == source || dest->erased.active ||
        dest->parts[0] || dest->parts[1] || !source->erased.active ||
        !source->ops.clone || !source->ops.drop)
        return DS_INVALID;
    void *parts[2] = {0};
    int status =
        source->ops.clone(source->ops.context, source->parts[0], &parts[0]);
    if (status != DS_OK)
        return status;
    status =
        source->ops.clone(source->ops.context, source->parts[1], &parts[1]);
    if (status != DS_OK)
    {
#if DS_EXT_MUTANT != 7
        source->ops.drop(source->ops.context, parts[0]);
#endif
        return status;
    }
    struct ds_clone_ops previous = dest->ops;
    dest->ops = source->ops;
    status = ds_clone_capture_adopt(dest, d_small_data(&source->erased.capture),
                                    source->erased.capture.length,
                                    source->erased.call, parts[0], parts[1]);
    if (status != DS_OK)
    {
        source->ops.drop(source->ops.context, parts[1]);
        source->ops.drop(source->ops.context, parts[0]);
        dest->ops = previous;
        return status;
    }
#if DS_EXT_MUTANT == 8
    dest->parts[0] = source->parts[0];
#endif
    return DS_OK;
}

void ds_clone_capture_drop(struct ds_clone_capture *c)
{
    ds_erased_drop(&c->erased);
}
