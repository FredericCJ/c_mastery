#include "identity.h"

int ai_init(struct ai_pool *p, unsigned bits)
{
    (void)p;
    (void)bits;
    return AI_UNIMPLEMENTED;
}

int ai_take(struct ai_pool *p, uint32_t *out)
{
    (void)p;
    (void)out;
    return AI_UNIMPLEMENTED;
}

int ai_valid(const struct ai_pool *p, uint32_t h)
{
    (void)p;
    (void)h;
    return 0;
}

int ai_release(struct ai_pool *p, uint32_t h)
{
    (void)p;
    (void)h;
    return AI_UNIMPLEMENTED;
}

int ai_word_cas(_Atomic(uint32_t) *p, uint32_t *e, uint32_t d)
{
    (void)p;
    (void)e;
    (void)d;
    return 0;
}
