#include "opaque.h"

struct efr_opaque
{
    er_fraction value;
    struct efr_memory memory;
};

er_code efr_opaque_make(struct efr_memory memory, uint32_t n, uint32_t d,
                        struct efr_opaque **out)
{
    if (!out || *out || !memory.take || !memory.release)
    {
        return ER_INVALID;
    }
    er_fraction value;
    if (er_refine(n, d, &value) != ER_OK)
    {
        return ER_INVALID;
    }
    struct efr_opaque *p = memory.take(memory.context, sizeof *p);
    if (!p)
    {
        return ER_IO;
    }
    p->value = value;
    p->memory = memory;
    *out = p;
    return ER_OK;
}

er_code efr_opaque_consume(const struct efr_opaque *p, uint32_t *out)
{
    if (!p || !out)
    {
        return ER_INVALID;
    }
    *out = p->value.numerator / p->value.denominator;
    return ER_OK;
}

void efr_opaque_drop(struct efr_opaque **owner)
{
    struct efr_opaque *p = *owner;
    if (!p)
    {
        return;
    }
    struct efr_memory memory = p->memory;
    *owner = NULL;
    memory.release(memory.context, p);
}
