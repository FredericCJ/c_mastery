#include "checkpoint_operation.h"
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
static enum cpo_status unlock(struct cp_operation *o, enum cpo_status result)
{
    return pthread_mutex_unlock(&o->mutex) ? CPO_ERROR : result;
}

enum cpo_status cpo_init(struct cp_operation *o, unsigned limit, void *context,
                         cpo_allocate_fn a, cpo_release_fn f, cpo_report_fn r)
{
    if (!o || !limit || !a || !f || !r || pthread_mutex_init(&o->mutex, 0))
        return CPO_ERROR;
    o->bytes = 0;
    o->length = 0;
    o->generation = 1;
    o->limit = limit;
    o->state = CPO_IDLE;
    o->report_failed = 0;
    o->context = context;
    o->allocate = a;
    o->release = f;
    o->report = r;
    return CPO_OK;
}

enum cpo_status cpo_begin(struct cp_operation *o, const unsigned char *input,
                          size_t n, unsigned *generation)
{
    if (!o || !generation || !input || !n || n > 1024 ||
        pthread_mutex_lock(&o->mutex))
        return CPO_ERROR;
    if (o->state == CPO_DEAD)
        return unlock(o, CPO_RETIRED);
    if (o->state != CPO_IDLE)
        return unlock(o, CPO_BUSY);
    unsigned char *copy = o->allocate(o->context, n);
    if (!copy)
        return unlock(o, CPO_NOMEM);
    for (size_t i = 0; i < n; ++i)
        copy[i] = input[i];
    o->bytes = copy;
    o->length = n;
    o->report_failed = 0;
    o->state = CPO_ACTIVE;
    *generation = o->generation;
    return unlock(o, CPO_OK);
}

enum cpo_status cpo_cancel(struct cp_operation *o, unsigned generation)
{
    if (!o || pthread_mutex_lock(&o->mutex))
        return CPO_ERROR;
    if (generation != o->generation ||
        (o->state != CPO_ACTIVE && o->state != CPO_CANCELLING))
        return unlock(o, CPO_STALE);
    o->state = CPO_CANCELLING;
    return unlock(o, CPO_PENDING);
}

enum cpo_status cpo_complete(struct cp_operation *o, unsigned generation,
                             int quiescent)
{
    if (!o || pthread_mutex_lock(&o->mutex))
        return CPO_ERROR;
    if ((CP_MUTANT != 32 && generation != o->generation) ||
        (o->state != CPO_ACTIVE && o->state != CPO_CANCELLING))
        return unlock(o, CPO_STALE);
    if (!quiescent && CP_MUTANT != 31)
        return unlock(o, CPO_PENDING);
    int cancelled = o->state == CPO_CANCELLING;
    o->state = CPO_REPORTING;
    if (unlock(o, CPO_OK) != CPO_OK)
        return CPO_ERROR;
    /* Reporter may observe copied bytes, but re-entry is rejected while REPORTING. */
    int report_failed =
        o->report(o->context, generation, o->bytes, o->length, cancelled);
    o->release(o->context, o->bytes);
    if (pthread_mutex_lock(&o->mutex))
        return CPO_ERROR;
    o->bytes = 0;
    o->length = 0;
    o->report_failed = report_failed;
    if (o->generation == o->limit)
        o->state = CPO_DEAD;
    else
    {
        ++o->generation;
        o->state = CPO_IDLE;
    }
    return unlock(o, CPO_OK);
}

enum cpo_status cpo_destroy(struct cp_operation *o)
{
    if (!o || pthread_mutex_lock(&o->mutex))
        return CPO_ERROR;
    int idle = o->state == CPO_IDLE || o->state == CPO_DEAD;
    enum cpo_status result = unlock(o, idle ? CPO_OK : CPO_PENDING);
    if (result != CPO_OK)
        return result;
    return pthread_mutex_destroy(&o->mutex) ? CPO_ERROR : CPO_OK;
}
