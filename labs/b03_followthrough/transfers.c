#include "follow.h"

/* Learner exercise: intentionally unimplemented, including changed conditions. */
int bf_turn(struct bf_round *a, b3_write_fn b[2], void *c[2], unsigned d,
            unsigned *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return B3_FAILED;
}

int bf_message(struct bf_message *a, unsigned b, const unsigned char *c,
               unsigned d, unsigned e, int f, unsigned *g)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    return B3_FAILED;
}

int bf_start(struct bf_operation *a)
{
    (void)a;
    return B3_FAILED;
}

int bf_event(struct bf_operation *a, unsigned b, enum bf_event c)
{
    (void)a;
    (void)b;
    (void)c;
    return B3_FAILED;
}

int bf_clear_read(struct bf_register *a, unsigned *b)
{
    (void)a;
    (void)b;
    return B3_FAILED;
}

int bf_w0c(struct bf_register *a, unsigned b)
{
    (void)a;
    (void)b;
    return B3_FAILED;
}

int bf_coherent(bf_read_half_fn a, void *b, unsigned c, uint16_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return B3_FAILED;
}

int bf_due16(uint16_t a, uint16_t b, uint16_t c, int *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return B3_FAILED;
}

int bf_ticks(unsigned a, unsigned b, unsigned c, unsigned *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return B3_FAILED;
}
