#include "contract_modes.h"
#include <stdlib.h>
#ifndef ECM_MUTATION
#define ECM_MUTATION 0
#endif
static int mutex_init(ecm_mutex *m)
{
#ifdef ECM_POSIX_THREADS
    return pthread_mutex_init(m, NULL) == 0;
#else
    return mtx_init(m, mtx_plain) == thrd_success;
#endif
}

static int mutex_lock(ecm_mutex *m)
{
#ifdef ECM_POSIX_THREADS
    return pthread_mutex_lock(m) == 0;
#else
    return mtx_lock(m) == thrd_success;
#endif
}

static int mutex_unlock(ecm_mutex *m)
{
#ifdef ECM_POSIX_THREADS
    return pthread_mutex_unlock(m) == 0;
#else
    return mtx_unlock(m) == thrd_success;
#endif
}

er_code ecm_init(struct ecm_buffer *b, unsigned char *data, size_t n,
                 size_t reports)
{
    if (!b || n > 8 || (n && !data))
    {
        return ER_INVALID;
    }
    *b = (struct ecm_buffer){data, n, reports};
    return ER_OK;
}

static void unchecked_write(unsigned char *p, size_t index, unsigned char value)
{
    p[index] = value;
}

er_code ecm_write(struct ecm_buffer *b, er_mode mode, size_t index,
                  unsigned char value)
{
    if (!b || mode < ER_OFF || mode > ER_ENFORCE)
    {
        return ER_INVALID;
    }
    if (index >= b->length)
    {
        /* This control is selected only with eight actual bytes and a shorter view. */
        if (ECM_MUTATION == 1 && mode == ER_OFF && index < 8)
        {
            unchecked_write(b->data, index, value);
            return ER_OK;
        }
        if (mode != ER_OFF)
        {
            if (ECM_MUTATION == 2)
            {
                ++b->reports;
            }
            if (ECM_MUTATION != 2 && b->reports == SIZE_MAX)
            {
                return ER_RANGE;
            }
            if (ECM_MUTATION != 2 && ECM_MUTATION != 3)
            {
                ++b->reports;
            }
        }
        return ER_CONTRACT;
    }
    if (ECM_MUTATION == 4 && mode == ER_ENFORCE)
    {
        return ER_CONTRACT;
    }
    unchecked_write(b->data, index, value);
    return ER_OK;
}

er_code ecm_shared_init(struct ecm_shared *s, unsigned char *data, size_t n,
                        int counter, size_t reports)
{
    if (!s || counter < 0 || counter > 100 || n > 8 || (n && !data))
    {
        return ER_INVALID;
    }
    if (!mutex_init(&s->mutex))
    {
        return ER_IO;
    }
    s->buffer = (struct ecm_buffer){data, n, reports};
    s->counter = counter;
    return ER_OK;
}

static void unlock(struct ecm_shared *s)
{
    if (!mutex_unlock(&s->mutex))
    {
        abort();
    }
}

er_code ecm_shared_write(struct ecm_shared *s, er_mode mode, size_t i,
                         unsigned char value)
{
    if (!s)
    {
        return ER_INVALID;
    }
    if (!mutex_lock(&s->mutex))
    {
        return ER_IO;
    }
    er_code code = ecm_write(&s->buffer, mode, i, value);
    unlock(s);
    return code;
}

er_code ecm_shared_counter(struct ecm_shared *s, er_mode mode, int amount)
{
    if (!s)
    {
        return ER_INVALID;
    }
    if (!mutex_lock(&s->mutex))
    {
        return ER_IO;
    }
    size_t private_reports = s->buffer.reports;
    er_code code = er_contract_counter(mode, amount, &s->counter,
                                       ECM_MUTATION == 5 ? &private_reports
                                                         : &s->buffer.reports);
    unlock(s);
    return code;
}

er_code ecm_shared_snapshot(struct ecm_shared *s, struct ecm_snapshot *out)
{
    if (!s || !out)
    {
        return ER_INVALID;
    }
    if (!mutex_lock(&s->mutex))
    {
        return ER_IO;
    }
    struct ecm_snapshot value = {.counter = s->counter,
                                 .reports = s->buffer.reports,
                                 .length = s->buffer.length};
    for (size_t i = 0; i < value.length; ++i)
    {
        value.bytes[i] = s->buffer.data[i];
    }
    unlock(s);
    *out = value;
    return ER_OK;
}

void ecm_shared_destroy(struct ecm_shared *s)
{
#ifdef ECM_POSIX_THREADS
    if (pthread_mutex_destroy(&s->mutex) != 0)
    {
        abort();
    }
#else
    mtx_destroy(&s->mutex);
#endif
}
