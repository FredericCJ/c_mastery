#include "ring.h"

/* Fill every contract from permitted primitives; this file compiles but must
 * fail semantic acceptance. Implement the target-independent ownership core. */
int en_range(uint32_t a, size_t b)
{
    (void)a;
    (void)b;
    return ED_INVALID;
}

int en_init(struct en_ring *a, struct en_memory *b, uint32_t c,
            const struct en_bus *d, struct ep_phy *e, const unsigned char f[6])
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return ED_INVALID;
}

int en_reset_begin(struct en_ring *a, uint32_t b, unsigned c, uint32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return ED_INVALID;
}

int en_reset_step(struct en_ring *a, uint32_t b)
{
    (void)a;
    (void)b;
    return ED_INVALID;
}

int en_start(struct en_ring *a)
{
    (void)a;
    return ED_INVALID;
}

int en_tx_copy(struct en_ring *a, const void *b, size_t c, struct en_ticket *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return ED_INVALID;
}

int en_tx_publish(struct en_ring *a, struct en_ticket b)
{
    (void)a;
    (void)b;
    return ED_INVALID;
}

int en_tx_reap(struct en_ring *a, struct en_ticket b, uint32_t *c)
{
    (void)a;
    (void)b;
    (void)c;
    return ED_INVALID;
}

int en_rx_take(struct en_ring *a, struct en_frame *b)
{
    (void)a;
    (void)b;
    return ED_INVALID;
}

int en_rx_release(struct en_ring *a, struct en_ticket b)
{
    (void)a;
    (void)b;
    return ED_INVALID;
}

int en_status(struct en_ring *a, uint32_t *b)
{
    (void)a;
    (void)b;
    return ED_INVALID;
}

int en_stop_begin(struct en_ring *a, uint32_t b, unsigned c, uint32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return ED_INVALID;
}

int en_stop_step(struct en_ring *a, uint32_t b)
{
    (void)a;
    (void)b;
    return ED_INVALID;
}
