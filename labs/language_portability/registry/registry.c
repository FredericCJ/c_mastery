#include "registry.h"

int lr_dispatch(uint32_t id, int x, int *out)
{
    (void)id;
    (void)x;
    (void)out;
    return -1;
}

int lr_name(uint32_t id, const char **out)
{
    (void)id;
    (void)out;
    return -1;
}

int lr_id(const char *name, uint32_t *out)
{
    (void)name;
    (void)out;
    return -1;
}

int lr_at(size_t i, uint32_t *id, const char **name)
{
    (void)i;
    (void)id;
    (void)name;
    return -1;
}

size_t lr_count(void)
{
    return 0;
}

size_t lr_probes(uint32_t id)
{
    (void)id;
    return 0;
}
