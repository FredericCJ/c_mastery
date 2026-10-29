#include "session_v2.h"

/* Intentional learner scaffold. No private representation or solution here. */
int ac_typed_v1_set(struct ac_typed_v1 *s, uint32_t i, uint32_t n)
{
    (void)s;
    (void)i;
    (void)n;
    return 1;
}

int ac_typed_v1_get(const struct ac_typed_v1 *s, struct ac_exposed_v1 *o)
{
    (void)s;
    (void)o;
    return 1;
}

int ac_typed_v2_set(struct ac_typed_v2 *s, uint32_t i, uint64_t n)
{
    (void)s;
    (void)i;
    (void)n;
    return 1;
}

int ac_typed_v2_get(const struct ac_typed_v2 *s, struct ac_exposed_v2 *o)
{
    (void)s;
    (void)o;
    return 1;
}

int ac_serialized_set(unsigned char o[12], uint32_t i, uint64_t n)
{
    (void)o;
    (void)i;
    (void)n;
    return 1;
}

int ac_serialized_get(const unsigned char i[12], struct ac_exposed_v2 *o)
{
    (void)i;
    (void)o;
    return 1;
}
