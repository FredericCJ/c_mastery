#include "result.h"

int bf_number(int a, unsigned b, struct b2_result *c)
{
    (void)a;
    (void)b;
    (void)c;
    return B2_FAILED;
}

int bf_text(const unsigned char *a, size_t b, struct b2_result *c)
{
    (void)a;
    (void)b;
    (void)c;
    return B2_FAILED;
}

void bf_value_init(struct bf_value *v)
{
    v->tag = BF_EMPTY;
}

int bf_clone(struct bf_value *a, const unsigned char *b, size_t c,
             bf_alloc_fn d, bf_free_fn e, void *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return B2_FAILED;
}

int bf_move(struct bf_value *a, struct bf_value *b)
{
    (void)a;
    (void)b;
    return B2_FAILED;
}

int bf_take(struct bf_value *a, struct bf_bytes *b)
{
    (void)a;
    (void)b;
    return B2_FAILED;
}

void bf_bytes_drop(struct bf_bytes *a)
{
    (void)a;
}

void bf_reset(struct bf_value *a)
{
    (void)a;
}

int bf_value_render(const struct bf_value *a, unsigned char *b, size_t c,
                    size_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return B2_FAILED;
}

int bf_encode(const struct bf_wire *a, unsigned char *b, size_t c, size_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return B2_FAILED;
}

int bf_decode(const unsigned char *a, size_t b, struct bf_wire *c)
{
    (void)a;
    (void)b;
    (void)c;
    return B2_FAILED;
}
