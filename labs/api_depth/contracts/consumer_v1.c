#include "session_v1.h"
#include <stdio.h>
#include <stdlib.h>

struct allocator
{
    unsigned calls, releases, deny;
};

static void *allocate(void *p, size_t n, size_t align)
{
    struct allocator *a = p;
    ++a->calls;
    if (a->deny || align > _Alignof(max_align_t))
    {
        return NULL;
    }
    return malloc(n);
}

static void release(void *p, void *object)
{
    struct allocator *a = p;
    ++a->releases;
    free(object);
}

int main(void)
{
    struct allocator a = {0, 0, 1};
    struct ac_allocate alloc = {&a, allocate, release};
    struct ac_opaque *p = NULL;
    if (ac_opaque_create(17, 0, alloc, &p) != 2 || p || a.calls != 1 ||
        a.releases)
    {
        return 1;
    }
    a.deny = 0;
    if (ac_opaque_create(17, 0, alloc, &p) || !p)
    {
        return 2;
    }
    struct ac_exposed_v1 value;
    if (ac_opaque_get(p, &value) || value.identity != 17 || value.count)
    {
        return 3;
    }
    a.deny = 1;
    struct ac_opaque *unchanged = p;
    int rejected = ac_opaque_create(18, 1, alloc, &unchanged);
    if (rejected != 2 || unchanged != p)
    {
        ac_opaque_destroy(p);
        fputs("REPRESENTATION-CHECK:failed output changed\n", stderr);
        return 5;
    }
    ac_opaque_destroy(p);
    p = NULL;
    ac_opaque_destroy(p);
    if (a.calls != 3 || a.releases != 1)
    {
        return 4;
    }
    printf("REPRESENTATION-PASS:old consumer size=%zu align=%zu calls=%u "
           "releases=%u\n",
           ac_opaque_size(), ac_opaque_alignment(), a.calls, a.releases);
    return 0;
}
