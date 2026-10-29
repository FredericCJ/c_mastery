#include "dispatch.h"
#include <limits.h>
#ifndef DS_MODULE_MUTANT
#define DS_MODULE_MUTANT 0
#endif
/* A distinct private representation, linked without dispatch.c's DSModule definition. */
struct ds_module
{
    void *context;
    int (*read)(void *, int *);
    unsigned live;
};
static struct ds_module pool[2];

int ds_module_open(struct ds_service service, struct ds_module **out)
{
    if (!service.read || !out)
    {
        return DS_INVALID;
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        if (!pool[i].live)
        {
            pool[i] = (struct ds_module){service.context, service.read, 1};
            *out = &pool[i];
            return DS_OK;
        }
    }
    return DS_FULL;
}

int ds_module_twice(struct ds_module *m, int *out)
{
    if (!m || !out || !m->live)
    {
        return DS_INVALID;
    }
    int total = 0;
    for (unsigned i = 0; i < (DS_MODULE_MUTANT == 1 ? 1u : 2u); ++i)
    {
        int value = 0;
        int rc = m->read(m->context, &value);
        if (rc != DS_OK)
        {
            return rc;
        }
        if ((value > 0 && total > INT_MAX - value) ||
            (value < 0 && total < INT_MIN - value))
        {
            return DS_RANGE;
        }
        total += value;
        if (DS_MODULE_MUTANT == 2)
        {
            *out = total;
        }
    }
    *out = total;
    return DS_OK;
}

void ds_module_close(struct ds_module *m)
{
    m->live = 0;
    m->read = NULL;
    m->context = NULL;
}
