#include "waiting.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

static void ok(int e)
{
    if (e)
    {
        abort();
    }
}

int tw_barrier_init(struct tw_barrier *b, unsigned n)
{
    if (!n || n > 4)
    {
        return EINVAL;
    }
    int e = pthread_mutex_init(&b->mutex, NULL);
    if (e)
    {
        return e;
    }
    e = pthread_cond_init(&b->condition, NULL);
    if (e)
    {
        ok(pthread_mutex_destroy(&b->mutex));
        return e;
    }
    b->expected = n;
    b->arrived = b->generation = b->broken = 0;
    return 0;
}

int tw_barrier_wait(struct tw_barrier *b)
{
    ok(pthread_mutex_lock(&b->mutex));
    unsigned generation = b->generation;
    int e = 0;
    if (b->broken)
    {
        e = ECANCELED;
    }
    else if (b->generation == UINT_MAX)
    {
        b->broken = 1;
        ok(pthread_cond_broadcast(&b->condition));
        e = EOVERFLOW;
    }
    else if (++b->arrived == b->expected)
    {
        b->arrived = 0;
        b->generation++;
        ok(pthread_cond_broadcast(&b->condition));
    }
    else
    {
        while (b->generation == generation && !b->broken)
        {
            ok(pthread_cond_wait(&b->condition, &b->mutex));
        }
        if (b->broken)
        {
            e = ECANCELED;
        }
    }
#ifdef TW_MUTANT7
    if (e == ECANCELED)
    {
        e = 0;
    }
#endif
    ok(pthread_mutex_unlock(&b->mutex));
    return e;
}

int tw_barrier_break(struct tw_barrier *b)
{
    ok(pthread_mutex_lock(&b->mutex));
    b->broken = 1;
    ok(pthread_cond_broadcast(&b->condition));
    ok(pthread_mutex_unlock(&b->mutex));
    return 0;
}

void tw_barrier_destroy(struct tw_barrier *b)
{
    ok(pthread_cond_destroy(&b->condition));
    ok(pthread_mutex_destroy(&b->mutex));
}

int tw_permits_init(struct tw_permits *p)
{
    int e = pthread_mutex_init(&p->mutex, NULL);
    if (!e)
    {
        p->mask = p->settlements = 0;
    }
    return e;
}

int tw_permit_take(struct tw_permits *p, unsigned *token)
{
    int e = EAGAIN;
    ok(pthread_mutex_lock(&p->mutex));
    for (unsigned i = 0; i < 3; i++)
    {
        if (!(p->mask & (1u << i)))
        {
            p->mask |= 1u << i;
            *token = i;
            e = 0;
            break;
        }
    }
    ok(pthread_mutex_unlock(&p->mutex));
    return e;
}

int tw_permit_release(struct tw_permits *p, unsigned token)
{
    if (token >= 3)
    {
        return EINVAL;
    }
    int e = 0;
    ok(pthread_mutex_lock(&p->mutex));
    if (!(p->mask & (1u << token)))
    {
        e = EALREADY;
    }
    else
    {
        p->mask &= ~(1u << token);
        p->settlements++;
    }
    ok(pthread_mutex_unlock(&p->mutex));
    return e;
}

void tw_permits_destroy(struct tw_permits *p)
{
    if (p->mask)
    {
        abort();
    }
    ok(pthread_mutex_destroy(&p->mutex));
}

int tw_reader_admit(unsigned readers, unsigned writer, unsigned waiting,
                    int preference)
{
    (void)readers;
#ifdef TW_MUTANT8
    (void)waiting;
    (void)preference;
    return !writer;
#else
    return !writer && (!preference || !waiting);
#endif
}

int tw_writer_admit(unsigned readers, unsigned writer)
{
    return !readers && !writer;
}

int tw_select(unsigned requirement)
{
#ifdef TW_MUTANT9
    (void)requirement;
    return TW_PERMITS;
#else
    return requirement < 4 ? (int)requirement : -1;
#endif
}
