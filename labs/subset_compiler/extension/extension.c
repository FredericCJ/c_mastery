#include "extension.h"

/* Learner implementation: rejection stubs compile; they cannot pass semantic gates. */
int ce_compile(const char *s, size_t n, struct ce_program *p,
               struct ce_diagnostic *d)
{
    (void)s;
    (void)n;
    (void)p;
    (void)d;
    return 0;
}

int ce_verify(const struct ce_program *p, unsigned h[CE_CODE])
{
    (void)p;
    (void)h;
    return 0;
}

int ce_run(const struct ce_program *p, uint32_t x, unsigned *f, uint32_t *out)
{
    (void)p;
    (void)x;
    (void)f;
    (void)out;
    return 0;
}

int ce_emit(const struct ce_program *p, unsigned t, char *out, size_t n)
{
    (void)p;
    (void)t;
    (void)out;
    (void)n;
    return 0;
}
