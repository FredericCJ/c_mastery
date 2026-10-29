#include "dispatch_extensions.h"

struct ds_int_interface ds_int_interface(const struct ds_int_receiver *r)
{
    (void)r;
    return (struct ds_int_interface){0};
}

struct ds_byte_interface ds_byte_interface(const struct ds_bytes_receiver *r)
{
    (void)r;
    return (struct ds_byte_interface){0};
}

struct ds_progress ds_read_progress(struct ds_reader r, unsigned char *p,
                                    size_t n)
{
    (void)r;
    (void)p;
    (void)n;
    return (struct ds_progress){DS_FAILED, 0};
}

int ds_borrow_closure_init(struct ds_borrow_closure *c, const int32_t *p,
                           size_t n, unsigned once)
{
    (void)c;
    (void)p;
    (void)n;
    (void)once;
    return DS_FAILED;
}

int ds_borrow_closure_call(struct ds_borrow_closure *c, int a, int *out)
{
    (void)c;
    (void)a;
    (void)out;
    return DS_FAILED;
}

int ds_owned_closure_init(struct ds_owned_closure *c, struct ds_resource r)
{
    (void)c;
    (void)r;
    return DS_FAILED;
}

int ds_owned_closure_call(struct ds_owned_closure *c, ds_owned_body_fn b, int a,
                          struct ds_owned_result *out)
{
    (void)c;
    (void)b;
    (void)a;
    (void)out;
    return DS_FAILED;
}

void ds_owned_closure_drop(struct ds_owned_closure *c)
{
    (void)c;
}

void ds_clone_capture_init(struct ds_clone_capture *c, struct d_allocator a,
                           struct ds_clone_ops o)
{
    (void)c;
    (void)a;
    (void)o;
}

int ds_clone_capture_adopt(struct ds_clone_capture *c, const unsigned char *p,
                           size_t n, ds_thunk_fn call, void *a, void *b)
{
    (void)c;
    (void)p;
    (void)n;
    (void)call;
    (void)a;
    (void)b;
    return DS_FAILED;
}

int ds_clone_capture(struct ds_clone_capture *d,
                     const struct ds_clone_capture *s)
{
    (void)d;
    (void)s;
    return DS_FAILED;
}

void ds_clone_capture_drop(struct ds_clone_capture *c)
{
    (void)c;
}
