#include "bringup.h"

/* Intentionally unfinished learner artifact. Implement against CONTRACTS.md. */
int bu_contains(struct bu_range a, struct bu_range b)
{
    (void)a;
    (void)b;
    return 0;
}

int bu_overlap(struct bu_range a, struct bu_range b)
{
    (void)a;
    (void)b;
    return 0;
}

enum bu_result bu_vectors(const unsigned char *a, size_t b, unsigned c,
                          uint32_t d, struct bu_range e, struct bu_range f,
                          struct bu_vectors *g)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    return BU_UNSUPPORTED;
}

enum bu_result bu_memory(unsigned char *a, size_t b, const unsigned char *c,
                         size_t d, const struct bu_copy *e, size_t f,
                         const struct bu_range *g, size_t h, struct bu_range i)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    (void)h;
    (void)i;
    return BU_UNSUPPORTED;
}

int bu_f439_dma(struct bu_range a)
{
    (void)a;
    return 0;
}

enum bu_result bu_clock_start(struct bu_clock *a, unsigned b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return BU_UNSUPPORTED;
}

enum bu_result bu_clock_step(struct bu_clock *a, unsigned b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return BU_UNSUPPORTED;
}

enum bu_result bu_clock_reconfigure(struct bu_clock *a, unsigned b)
{
    (void)a;
    (void)b;
    return BU_UNSUPPORTED;
}

enum bu_result bu_clock_failure(struct bu_clock *a)
{
    (void)a;
    return BU_UNSUPPORTED;
}

enum bu_result bu_hooks(const struct bu_hook *a, unsigned b, unsigned c,
                        unsigned *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return BU_UNSUPPORTED;
}

enum bu_result bu_decode(const unsigned char *a, size_t b, struct bu_record *c)
{
    (void)a;
    (void)b;
    (void)c;
    return BU_UNSUPPORTED;
}

enum bu_result bu_store(unsigned char *a, size_t b, const struct bu_record *c,
                        unsigned d, unsigned *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return BU_UNSUPPORTED;
}

enum bu_result bu_select(const unsigned char a[2][BU_RECORD], unsigned *b)
{
    (void)a;
    (void)b;
    return BU_UNSUPPORTED;
}

enum bu_result bu_migrate(const struct bu_record *a, struct bu_record *b)
{
    (void)a;
    (void)b;
    return BU_UNSUPPORTED;
}

enum bu_result bu_handoff(const struct bu_handoff *a, struct bu_range b,
                          struct bu_range c)
{
    (void)a;
    (void)b;
    (void)c;
    return BU_UNSUPPORTED;
}
