#include "core.h"

unsigned kl_unwind(unsigned mask)
{
    return 0;
}

int kl_copy(unsigned char *dst, size_t cap, const unsigned char *src, size_t n,
            size_t fault_after)
{
    return 0;
}

int kl_ref(struct kl_ref *r, unsigned op)
{
    return 0;
}

int kl_lock_allowed(unsigned context, unsigned kind, unsigned rt)
{
    return 0;
}

int kl_work(struct kl_work *w, unsigned event)
{
    return 0;
}

int kl_grace(struct kl_grace *g, unsigned event, unsigned id)
{
    return 0;
}

int kl_decode(const unsigned char *p, size_t n, unsigned *out)
{
    return 0;
}

int kl_can_unload(unsigned handles, unsigned callbacks, unsigned closed)
{
    return 0;
}
