#include "checkpoint_firmware.h"

int cpf_ring_init(struct cpf_ring *r, unsigned n)
{
    (void)r;
    (void)n;
    return 0;
}

int cpf_irq_push(struct cpf_ring *r, struct cpf_sample s)
{
    (void)r;
    (void)s;
    return 0;
}

int cpf_take(struct cpf_ring *r, struct cpf_sample *s)
{
    (void)r;
    (void)s;
    return 0;
}

enum cp_status cpf_frame(const struct cpf_sample *s, unsigned char *b, size_t n,
                         size_t *w)
{
    (void)s;
    (void)b;
    (void)n;
    (void)w;
    return CP_INVALID;
}

void cpf_tx_init(struct cpf_tx *t, unsigned l, unsigned r)
{
    (void)t;
    (void)l;
    (void)r;
}

enum cpf_status cpf_tx_begin(struct cpf_tx *t, const struct cp_message *m,
                             int d)
{
    (void)t;
    (void)m;
    (void)d;
    return CPF_INVALID;
}

enum cpf_status cpf_tx_poll(struct cpf_tx *t, unsigned g, int e, int c,
                            cpf_write_fn w, void *x)
{
    (void)t;
    (void)g;
    (void)e;
    (void)c;
    (void)w;
    (void)x;
    return CPF_INVALID;
}

enum cpf_status cpf_tx_dma(struct cpf_tx *t, unsigned g, int e, size_t n, int c,
                           int f)
{
    (void)t;
    (void)g;
    (void)e;
    (void)n;
    (void)c;
    (void)f;
    return CPF_INVALID;
}

enum cpf_status cpf_tx_timeout(struct cpf_tx *t, unsigned g, uint32_t c)
{
    (void)t;
    (void)g;
    (void)c;
    return CPF_INVALID;
}

enum cpf_status cpf_tx_recover(struct cpf_tx *t, unsigned g, int d, int u)
{
    (void)t;
    (void)g;
    (void)d;
    (void)u;
    return CPF_INVALID;
}

enum cpf_status cpf_tx_next(struct cpf_tx *t)
{
    (void)t;
    return CPF_INVALID;
}

int cpf_clock_init(struct cpf_clock *c)
{
    (void)c;
    return 0;
}

int cpf_clock_write(struct cpf_clock *c, uint64_t v)
{
    (void)c;
    (void)v;
    return 0;
}

int cpf_clock_read(const struct cpf_clock *c, unsigned n, uint64_t *v)
{
    (void)c;
    (void)n;
    (void)v;
    return 0;
}

int cpf_elapsed16(uint16_t a, uint16_t b, uint16_t *v)
{
    (void)a;
    (void)b;
    (void)v;
    return 0;
}
