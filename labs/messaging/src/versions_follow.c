#include "versions.h"

enum msg_status mfv_init(struct mfv_store *s, struct mfl_allocator a,
                         struct mfo_value *v)
{
    (void)s;
    (void)a;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfv_snapshot(struct mfv_store *s, struct msg_snapshot *p)
{
    (void)s;
    (void)p;
    return MSG_INVALID;
}

enum msg_status mfv_read(struct mfv_store *s, struct msg_snapshot p,
                         struct mfl_shared **v)
{
    (void)s;
    (void)p;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfv_drop_snapshot(struct mfv_store *s, struct msg_snapshot p)
{
    (void)s;
    (void)p;
    return MSG_INVALID;
}

enum msg_status mfv_publish(struct mfv_store *s, mfv_clone_fn c, void *x,
                            const void *v)
{
    (void)s;
    (void)c;
    (void)x;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfv_destroy(struct mfv_store *s)
{
    (void)s;
    return MSG_INVALID;
}

enum msg_status mfk_init(struct mfk_store *s, unsigned a, unsigned b)
{
    (void)s;
    (void)a;
    (void)b;
    return MSG_INVALID;
}

enum msg_status mfk_begin(struct mfk_store *s, struct mfk_txn *t)
{
    (void)s;
    (void)t;
    return MSG_INVALID;
}

enum msg_status mfk_read(struct mfk_txn *t, unsigned k, unsigned *v)
{
    (void)t;
    (void)k;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfk_nonzero(struct mfk_txn *t, unsigned *v)
{
    (void)t;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfk_write(struct mfk_txn *t, unsigned k, unsigned v)
{
    (void)t;
    (void)k;
    (void)v;
    return MSG_INVALID;
}

enum msg_status mfk_commit(struct mfk_txn *t, bool p)
{
    (void)t;
    (void)p;
    return MSG_INVALID;
}

void mfk_destroy(struct mfk_store *s)
{
    (void)s;
}
