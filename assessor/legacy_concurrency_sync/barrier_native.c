#include "barrier_native.h"

int cdbn_init(struct cdb_native *p, unsigned parties)
{
    if (!p || !parties || parties > 4)
        return CMX_INVALID;
    *p = (struct cdb_native){.parties = parties};
    cmx_init(&p->kernel, CMX_FIXED);
    for (unsigned i = 0; i < parties; i++)
        if (cmx_task_add(&p->kernel, 5 + i, 0, 0x1000 + 256 * i, 256) != (int)i)
            return CMX_INVALID;
    if (cdb_init(&p->barrier, &p->kernel, (UINT32_C(1) << parties) - 1, 255) !=
        CMX_OK)
        return CMX_INVALID;
    for (unsigned i = 0; i < parties; i++)
        if (cdb_register(&p->barrier, i, &p->member[i]) != CMX_OK)
            return CMX_INVALID;
    if (pthread_mutex_init(&p->exclusion, 0))
        return CMX_INVALID;
    if (pthread_cond_init(&p->changed, 0))
    {
        (void)pthread_mutex_destroy(&p->exclusion);
        return CMX_INVALID;
    }
    p->live = 1;
    return CMX_OK;
}

int cdbn_wait(struct cdb_native *p, unsigned i)
{
    if (!p || !p->live || i >= p->parties || pthread_mutex_lock(&p->exclusion))
        return CMX_INVALID;
    struct cdb_ticket ticket;
    int result = cdb_arrive(p->member[i], cmx_forever(), &ticket);
    if (result == CMX_OK || result == CMX_BLOCKED)
    {
        p->arrivals++;
        if (result == CMX_OK && pthread_cond_broadcast(&p->changed))
            result = CMX_INVALID;
        if (result != CMX_INVALID)
        {
            unsigned generation;
            while ((result = cdb_result(ticket, &generation)) == CMX_BLOCKED)
                if (pthread_cond_wait(&p->changed, &p->exclusion))
                {
                    (void)cdb_cancel(ticket);
                    (void)pthread_cond_broadcast(&p->changed);
                    result = CMX_INVALID;
                    break;
                }
        }
    }
    if (pthread_mutex_unlock(&p->exclusion))
        return CMX_INVALID;
    return result;
}

int cdbn_destroy(struct cdb_native *p)
{
    if (!p || !p->live)
        return CMX_INVALID;
    int result = cdb_close(&p->barrier);
    if (result != CMX_OK)
        return result;
    for (unsigned i = 0; i < p->parties; i++)
    {
        result = cdb_release(&p->member[i]);
        if (result != CMX_OK)
            return result;
    }
    if (cdb_destroy(&p->barrier) != CMX_OK)
        return CMX_INVALID;
    if (pthread_cond_destroy(&p->changed) ||
        pthread_mutex_destroy(&p->exclusion))
        return CMX_INVALID;
    p->live = 0;
    return CMX_OK;
}

int cdbt_init(struct cdb_tree *t)
{
    if (!t)
        return CMX_INVALID;
    *t = (struct cdb_tree){0};
    int r = cdbn_init(&t->leaf[0], 2);
    if (r != CMX_OK)
        return r;
    r = cdbn_init(&t->leaf[1], 2);
    if (r != CMX_OK)
    {
        (void)cdbn_destroy(&t->leaf[0]);
        return r;
    }
    r = cdbn_init(&t->root, 2);
    if (r != CMX_OK)
    {
        (void)cdbn_destroy(&t->leaf[1]);
        (void)cdbn_destroy(&t->leaf[0]);
        return r;
    }
    t->live = 1;
    return CMX_OK;
}

int cdbt_wait(struct cdb_tree *t, unsigned i)
{
    if (!t || !t->live || i >= 4)
        return CMX_INVALID;
    unsigned group = i / 2, local = i % 2;
    int r = cdbn_wait(&t->leaf[group], local);
    if (r != CMX_OK)
        return r;
    if (!local)
    {
        r = cdbn_wait(&t->root, group);
        if (r != CMX_OK)
            return r;
    }
    return cdbn_wait(&t->leaf[group], local);
}

int cdbt_destroy(struct cdb_tree *t)
{
    if (!t || !t->live)
        return CMX_INVALID;
    int a = cdbn_destroy(&t->root), b = cdbn_destroy(&t->leaf[0]),
        c = cdbn_destroy(&t->leaf[1]);
    if (a != CMX_OK || b != CMX_OK || c != CMX_OK)
        return CMX_INVALID;
    t->live = 0;
    return CMX_OK;
}
