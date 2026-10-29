#include "transactions.h"
#ifndef ST_MUTANT
#define ST_MUTANT 0
#endif
static int valid(struct st_txn *t)
{
    return t && t->engine && !t->engine->poisoned;
}

static int active(struct st_txn *t)
{
    return valid(t) && t->engine->active;
}

int st_init(struct st_txn *t, store *s)
{
    if (!t || !s || s->active || s->poisoned)
    {
        return STORE_STATE;
    }
    int r = store_check(s);
    if (r)
    {
        return r;
    }
    struct st_txn staged = {0};
    staged.engine = s;
    *t = staged;
    return STORE_OK;
}

int st_begin(struct st_txn *t)
{
    if (!valid(t) || t->saved)
    {
        return STORE_STATE;
    }
    return store_begin(t->engine);
}

int st_put(struct st_txn *t, const void *k, size_t n, const void *v, size_t z)
{
    if (!active(t))
    {
        return STORE_STATE;
    }
    store before = *t->engine;
    int r = store_put(t->engine, k, n, v, z);
    if (r && ST_MUTANT != 1)
    {
        *t->engine = before;
    }
    if (ST_MUTANT == 4 && r == STORE_OK)
    {
        uint8_t bytes[STORE_PAGE_SIZE];
        if (store_page_encode(1, t->engine->generation, &t->engine->scratch[1],
                              bytes) == STORE_OK)
        {
            (void)t->engine->io.write(t->engine->io.ctx, 3, bytes);
        }
    }
    return r;
}

int st_delete(struct st_txn *t, const void *k, size_t n)
{
    if (!active(t))
    {
        return STORE_STATE;
    }
    store before = *t->engine;
    int r = store_delete(t->engine, k, n);
    if (r)
    {
        *t->engine = before;
    }
    return r;
}

int st_savepoint(struct st_txn *t, struct st_savepoint *out)
{
    if (!active(t) || t->saved || !out)
    {
        return STORE_STATE;
    }
    if (t->serial == UINT64_MAX)
    {
        return STORE_FULL;
    }
    t->undo = *t->engine;
    t->serial++;
    t->saved = 1;
    struct st_savepoint s = {t, t->serial};
    *out = s;
    return STORE_OK;
}

static int matches(struct st_txn *t, struct st_savepoint s)
{
    return active(t) && t->saved && s.owner == t &&
           (ST_MUTANT == 2 || s.serial == t->serial);
}

int st_rollback(struct st_txn *t, struct st_savepoint s)
{
    if (!matches(t, s))
    {
        return STORE_STATE;
    }
    *t->engine = t->undo;
    t->saved = 0;
    return STORE_OK;
}

int st_release(struct st_txn *t, struct st_savepoint s)
{
    if (!matches(t, s))
    {
        return STORE_STATE;
    }
    t->saved = 0;
    return STORE_OK;
}

int st_commit(struct st_txn *t)
{
    if (!active(t))
    {
        return STORE_STATE;
    }
    int r = store_commit(t->engine);
    if (ST_MUTANT != 3)
    {
        t->saved = 0;
    }
    return r;
}

int st_abort(struct st_txn *t)
{
    if (!active(t))
    {
        return STORE_STATE;
    }
    store_abort(t->engine);
    t->saved = 0;
    return STORE_OK;
}
