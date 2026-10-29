#include "session_v1.h"
#ifndef AC_INTERNAL_WIDE
#define AC_INTERNAL_WIDE 0
#endif
struct ac_opaque
{
    uint32_t identity;
#if AC_INTERNAL_WIDE
    uint64_t count;
#else
    uint32_t count;
#endif
    struct ac_allocate allocator;
};

int ac_opaque_create(uint32_t id, uint32_t n, struct ac_allocate a,
                     struct ac_opaque **out)
{
    if (!out || !a.allocate || !a.release)
    {
        return 1;
    }
#ifdef AC_MUTANT_OUTPUT
    *out = NULL;
#endif
    struct ac_opaque *s =
        a.allocate(a.context, sizeof *s, _Alignof(struct ac_opaque));
    if (!s)
    {
        return 2;
    }
    s->identity = id;
    s->count = n;
    s->allocator = a;
    *out = s;
    return 0;
}

int ac_opaque_get(const struct ac_opaque *s, struct ac_exposed_v1 *out)
{
    if (!s || !out)
    {
        return 1;
    }
    *out = (struct ac_exposed_v1){s->identity, (uint32_t)s->count};
    return 0;
}

void ac_opaque_destroy(struct ac_opaque *s)
{
    if (s)
    {
        struct ac_allocate a = s->allocator;
        a.release(a.context, s);
    }
}

size_t ac_opaque_size(void)
{
    return sizeof(struct ac_opaque);
}

size_t ac_opaque_alignment(void)
{
    return _Alignof(struct ac_opaque);
}
