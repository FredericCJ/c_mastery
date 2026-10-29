#include "notifications.h"

/* Unsolved learner scaffold: preserve the actual CMX producer. */
void cdn_init(struct cd_notify *n, cmx_kernel *k)
{
    (void)n;
    (void)k;
}

int cdn_attach(struct cd_notify *n, unsigned t, unsigned m,
               struct cdn_handle *h)
{
    (void)n;
    (void)t;
    (void)m;
    (void)h;
    return CMX_INVALID;
}

int cdn_post(struct cdn_handle h, uint32_t n, struct cdn_post *p)
{
    (void)h;
    (void)n;
    (void)p;
    return CMX_INVALID;
}

int cdn_wait(struct cdn_handle h, uint32_t n, cmx_deadline d)
{
    (void)h;
    (void)n;
    (void)d;
    return CMX_INVALID;
}

int cdn_result(struct cdn_handle h, uint32_t *n)
{
    (void)h;
    (void)n;
    return CMX_INVALID;
}

int cdn_cancel(struct cdn_handle h)
{
    (void)h;
    return CMX_INVALID;
}

int cdn_close(struct cdn_handle h)
{
    (void)h;
    return CMX_INVALID;
}

int cdn_detach(struct cdn_handle h)
{
    (void)h;
    return CMX_INVALID;
}

int cdn_fault(struct cdn_handle h, struct cdn_fault *f)
{
    (void)h;
    (void)f;
    return CMX_INVALID;
}

int cdn_ack_fault(struct cdn_handle h, uint64_t s)
{
    (void)h;
    (void)s;
    return CMX_INVALID;
}

int cdn_invariant(const struct cd_notify *n)
{
    (void)n;
    return 0;
}
