#include "transactions.h"
#include <limits.h>
#ifndef MFT_MUTANT
#define MFT_MUTANT 0
#endif
static bool hook(mft_hook_fn h, void *ctx, enum mft_phase p, unsigned k)
{
    bool permitted = !h || h(ctx, p, k);
    return MFT_MUTANT == 76 ? true : permitted;
}

enum msg_status mft_commit_phase(struct msg_txn *t, mft_hook_fn h, void *ctx)
{
    if (!t || !t->active)
        return MSG_INVALID;
    struct msg_store *s = t->store;
    unsigned locked = 0, old[4] = {0}, stamp = 0;
    enum msg_status status = MSG_CONFLICT;
    for (unsigned i = 0; i < 4; i++)
        if (t->writes & (1u << i))
        {
            unsigned v = atomic_load(&s->cells[i].stamp);
            old[i] = v;
            if ((v & 1u) ||
                !atomic_compare_exchange_strong(&s->cells[i].stamp, &v, v | 1u))
                goto abort;
            locked |= 1u << i;
            if (!hook(h, ctx, MFT_LOCKED, i))
            {
                status = MSG_FAILED;
                goto abort;
            }
        }
    if (t->writes)
    {
        unsigned clock = atomic_load(&s->clock);
        do
        {
            if (clock == UINT_MAX / 2)
            {
                status = MSG_OVERFLOW;
                goto abort;
            }
        } while (!atomic_compare_exchange_weak(&s->clock, &clock, clock + 1));
        stamp = clock + 1;
        if (!hook(h, ctx, MFT_CLOCK, 4))
        {
            status = MSG_FAILED;
            goto abort;
        }
    }
    for (unsigned i = 0; i < 4; i++)
        if (t->reads & (1u << i))
        {
            unsigned v =
                (locked & (1u << i)) ? old[i] : atomic_load(&s->cells[i].stamp);
            if ((v & 1u) || (v >> 1) > t->snapshot)
                goto abort;
        }
    if (!hook(h, ctx, MFT_VALIDATED, 4) || !hook(h, ctx, MFT_PREPUBLISH, 4))
    {
        status = MSG_FAILED;
        goto abort;
    }
    /* Once payload publication starts, hooks only observe. They cannot abort. */
    for (unsigned i = 0; i < 4; i++)
        if (locked & (1u << i))
        {
            atomic_store(&s->cells[i].value, t->values[i]);
            (void)hook(h, ctx, MFT_PAYLOAD, i);
        }
    for (unsigned i = 0; i < 4; i++)
        if (locked & (1u << i))
        {
            atomic_store(&s->cells[i].stamp, stamp << 1);
            (void)hook(h, ctx, MFT_UNLOCKED, i);
        }
    t->active = false;
    (void)hook(h, ctx, MFT_FINISHED, 4);
    return MSG_OK;
abort:
    for (unsigned i = 0; i < 4; i++)
        if ((locked & (1u << i)) && MFT_MUTANT != 74)
            atomic_store(&s->cells[i].stamp, old[i]);
    t->active = false;
    return status;
}

static void cleanup(struct mft_txn *t)
{
    for (unsigned i = 0; i < 4; i++)
    {
        mfl_drop(&t->reads[i]);
        if (MFT_MUTANT != 75)
            mfl_drop(&t->writes[i]);
    }
}

enum msg_status mft_init(struct mft_store *s, struct mfl_allocator a)
{
    if (!s || !a.allocate || !a.deallocate)
        return MSG_INVALID;
    if (pthread_mutex_init(&s->mutex, NULL))
        return MSG_FAILED;
    msg_store_init(&s->core);
    s->allocator = a;
    s->log_count = s->active = 0;
    for (unsigned i = 0; i < 4; i++)
        s->nodes[i] = NULL;
    return MSG_OK;
}

enum msg_status mft_begin(struct mft_store *s, struct mft_txn *out)
{
    if (!s || !out)
        return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);
    if (s->active == UINT_MAX)
    {
        pthread_mutex_unlock(&s->mutex);
        return MSG_OVERFLOW;
    }
    struct mft_txn t = {.owner = s, .active = true};
    msg_begin(&s->core, &t.core);
    ++s->active;
    pthread_mutex_unlock(&s->mutex);
    *out = t;
    return MSG_OK;
}

void mft_abort(struct mft_txn *t)
{
    if (!t || !t->active)
        return;
    msg_abort(&t->core);
    t->active = false;
    pthread_mutex_lock(&t->owner->mutex);
    --t->owner->active;
    pthread_mutex_unlock(&t->owner->mutex);
    cleanup(t);
}

enum msg_status mft_read(struct mft_txn *t, unsigned k,
                         const struct mfo_value **out)
{
    if (!t || !out || !t->active || k > 3)
        return MSG_INVALID;
    struct mft_store *s = t->owner;
    pthread_mutex_lock(&s->mutex);
    unsigned present;
    enum msg_status status = msg_read(&t->core, k, &present);
    struct mfl_shared *node = NULL;
    if (status == MSG_OK)
    {
        node = (t->core.writes & (1u << k)) ? t->writes[k] : s->nodes[k];
        if (!(t->core.writes & (1u << k)) && !t->reads[k] && node)
        {
            status = MFT_MUTANT == 73 ? MSG_OK : mfl_retain(node);
            if (status == MSG_OK)
                t->reads[k] = node;
        }
        if (status == MSG_OK)
            *out = node ? mfl_borrow(node) : NULL;
    }
    pthread_mutex_unlock(&s->mutex);
    if (status != MSG_OK)
        mft_abort(t);
    return status;
}

enum msg_status mft_write(struct mft_txn *t, unsigned k, struct mfo_value *v)
{
    if (!t || !v || !t->active || k > 3)
        return MSG_INVALID;
    struct mfl_shared *node = NULL;
    enum msg_status status = mfl_make(t->owner->allocator, v, &node);
    if (status != MSG_OK)
        return status;
    status = msg_write(&t->core, k, 1);
    if (status != MSG_OK)
    {
        mfl_drop(&node);
        return status;
    }
    mfl_drop(&t->writes[k]);
    t->writes[k] = node;
    return MSG_OK;
}

enum msg_status mft_remove(struct mft_txn *t, unsigned k)
{
    if (!t || !t->active || k > 3)
        return MSG_INVALID;
    enum msg_status status = msg_write(&t->core, k, 0);
    if (status == MSG_OK)
        mfl_drop(&t->writes[k]);
    return status;
}

enum msg_status mft_log(struct mft_txn *t, unsigned event)
{
    if (!t || !t->active)
        return MSG_INVALID;
    if (t->log_count == 4)
        return MSG_FULL;
    t->log[t->log_count++] = event;
    if (MFT_MUTANT == 71)
    {
        pthread_mutex_lock(&t->owner->mutex);
        t->owner->log[t->owner->log_count++] = event;
        pthread_mutex_unlock(&t->owner->mutex);
    }
    return MSG_OK;
}

enum msg_status mft_commit(struct mft_txn *t, mft_hook_fn h, void *ctx)
{
    if (!t || !t->active)
        return MSG_INVALID;
    struct mft_store *s = t->owner;
    struct mfl_shared *old[4] = {0};
    pthread_mutex_lock(&s->mutex);
    enum msg_status status;
    if (t->log_count > 64 - s->log_count)
    {
        status = MSG_FULL;
        msg_abort(&t->core);
    }
    else
    {
        t->core.reads |= t->core.writes;
        status = mft_commit_phase(&t->core, h, ctx);
        if (status == MSG_OK)
        {
            for (unsigned i = 0; i < 4; i++)
                if (t->core.writes & (1u << i))
                {
                    old[i] = s->nodes[i];
                    s->nodes[i] = t->writes[i];
                    t->writes[i] = NULL;
                }
            if (MFT_MUTANT != 72)
                for (unsigned i = 0; i < t->log_count; i++)
                    s->log[s->log_count++] = t->log[i];
        }
    }
    --s->active;
    t->active = false;
    pthread_mutex_unlock(&s->mutex);
    for (unsigned i = 0; i < 4; i++)
        mfl_drop(&old[i]);
    cleanup(t);
    return status;
}

enum msg_status mft_run(struct mft_store *s, unsigned limit, mft_body_fn body,
                        void *ctx, unsigned *attempts)
{
    if (!s || !limit || !body || !attempts)
        return MSG_INVALID;
    *attempts = 0;
    enum msg_status status = MSG_CONFLICT;
    while (*attempts < limit)
    {
        struct mft_txn t;
        status = mft_begin(s, &t);
        if (status != MSG_OK)
            return status;
        ++*attempts;
        status = body(ctx, &t);
        if (status == MSG_OK)
            status = mft_commit(&t, NULL, NULL);
        else
            mft_abort(&t);
        if (status != MSG_CONFLICT)
            return status;
    }
    return status;
}

enum msg_status mft_destroy(struct mft_store *s)
{
    if (!s)
        return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);
    if (s->active)
    {
        pthread_mutex_unlock(&s->mutex);
        return MSG_BUSY;
    }
    struct mfl_shared *old[4];
    for (unsigned i = 0; i < 4; i++)
    {
        old[i] = s->nodes[i];
        s->nodes[i] = NULL;
    }
    pthread_mutex_unlock(&s->mutex);
    pthread_mutex_destroy(&s->mutex);
    for (unsigned i = 0; i < 4; i++)
        mfl_drop(&old[i]);
    return MSG_OK;
}
