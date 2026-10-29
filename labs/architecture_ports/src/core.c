#include "core.h"

int po_stack(uint64_t top, uint64_t bytes, uint64_t *out)
{
    return 0;
}

int po_saved(unsigned arch, unsigned reg)
{
    return 0;
}

int po_arg(unsigned index, unsigned *reg, unsigned *offset)
{
    return 0;
}

int po_save(struct po_context *d, unsigned a, const uint64_t *r)
{
    (void)d;
    (void)a;
    (void)r;
    return 0;
}

int po_restore(const struct po_context *s, unsigned a, uint64_t *r)
{
    (void)s;
    (void)a;
    (void)r;
    return 0;
}

int po_sign12(uint32_t r)
{
    (void)r;
    return 0;
}

int po_branch(int64_t d, unsigned b, unsigned a)
{
    (void)d;
    (void)b;
    (void)a;
    return 0;
}

int po_trap(struct po_status *s, unsigned f)
{
    (void)s;
    (void)f;
    return 0;
}

int po_return(struct po_status *s)
{
    (void)s;
    return 0;
}

int po_access(unsigned user, unsigned supervisor, unsigned sum, unsigned mxr,
              unsigned r, unsigned w, unsigned x, unsigned op)
{
    return 0;
}

int po_closure(unsigned required, unsigned provided)
{
    return 0;
}
