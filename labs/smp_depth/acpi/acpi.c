#include "acpi.h"

/* Implement checked wire parsing and transactional topology publication. */
int sa_parse(const void *p, size_t n, unsigned capacity, unsigned profile,
             struct sa_topology *out)
{
    (void)p;
    (void)n;
    (void)capacity;
    (void)profile;
    (void)out;
    return SA_INVALID;
}

int sa_discover(const void *p, size_t n, const struct sa_region *r, unsigned nr,
                unsigned c, unsigned mode, struct sa_topology *o)
{
    (void)p;
    (void)n;
    (void)r;
    (void)nr;
    (void)c;
    (void)mode;
    (void)o;
    return SA_INVALID;
}
