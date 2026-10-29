#include "device.h"

/* Intentional learner implementation boundary. */
int kf_pci(const uint8_t c[256], const struct kf_bar b[6], struct kf_pci *o)
{
    (void)c;
    (void)b;
    (void)o;
    return 0;
}

int kf_negotiate(uint64_t f, unsigned q, uint16_t n, const struct kf_pci *p,
                 uint64_t *o)
{
    (void)f;
    (void)q;
    (void)n;
    (void)p;
    (void)o;
    return 0;
}

void kf_queue_init(struct kf_queue *q, uint16_t i)
{
    (void)q;
    (void)i;
}

int kf_submit(struct kf_queue *q, unsigned t, uint64_t s, const uint8_t *b,
              struct kf_ticket *o)
{
    (void)q;
    (void)t;
    (void)s;
    (void)b;
    (void)o;
    return 0;
}

int kf_complete(struct kf_queue *q, uint16_t i, uint32_t h, uint8_t s)
{
    (void)q;
    (void)i;
    (void)h;
    (void)s;
    return 0;
}

int kf_timeout(struct kf_queue *q, struct kf_ticket t)
{
    (void)q;
    (void)t;
    return 0;
}

int kf_release(struct kf_queue *q, struct kf_ticket t)
{
    (void)q;
    (void)t;
    return 0;
}

int kf_reset(struct kf_queue *q, int a)
{
    (void)q;
    (void)a;
    return 0;
}

int kf_page_sector(uint64_t r, unsigned b, uint32_t p, uint64_t *o)
{
    (void)r;
    (void)b;
    (void)p;
    (void)o;
    return 0;
}

int kf_slice(uint64_t r, uint32_t b, uint32_t c, uint32_t l, uint32_t *o)
{
    (void)r;
    (void)b;
    (void)c;
    (void)l;
    (void)o;
    return 0;
}
