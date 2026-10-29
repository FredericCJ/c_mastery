#include "lifetime.h"

enum msg_status mfl_make(struct mfl_allocator a, struct mfo_value *v,
                         struct mfl_shared **out)
{
    (void)a;
    (void)v;
    (void)out;
    return MSG_FAILED;
}

enum msg_status mfl_retain(struct mfl_shared *p)
{
    (void)p;
    return MSG_INVALID;
}

void mfl_drop(struct mfl_shared **p)
{
    (void)p;
}

const struct mfo_value *mfl_borrow(const struct mfl_shared *p)
{
    (void)p;
    return NULL;
}

enum msg_status mfg_init(struct mfg_deque *q, struct mfl_allocator a, size_t n)
{
    (void)q;
    (void)a;
    (void)n;
    return MSG_INVALID;
}

enum msg_status mfg_push(struct mfg_deque *q, struct mfo_value *v)
{
    (void)q;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfg_pop(struct mfg_deque *q, struct mfl_shared **out)
{
    (void)q;
    (void)out;
    return MSG_INVALID;
}

enum msg_status mfg_steal(struct mfg_deque *q, struct mfl_shared **out,
                          mfg_pause_fn f, void *c)
{
    (void)q;
    (void)out;
    (void)f;
    (void)c;
    return MSG_INVALID;
}

enum msg_status mfg_destroy(struct mfg_deque *q)
{
    (void)q;
    return MSG_INVALID;
}

enum msg_status mfp_init(struct mfp_pointer *p, struct mfl_shared **v)
{
    (void)p;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfp_snapshot(struct mfp_pointer *p, struct mfl_shared **v)
{
    (void)p;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfp_replace(struct mfp_pointer *p, struct mfl_shared **v)
{
    (void)p;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfp_destroy(struct mfp_pointer *p)
{
    (void)p;
    return MSG_INVALID;
}
