#include "session.h"

int es_init(struct es_peer *a, unsigned b, unsigned c, uint16_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return ES_STUB;
}

int es_emit(struct es_peer *a, unsigned b, uint16_t c, uint16_t d, uint16_t e,
            const uint8_t *f, size_t g, uint16_t h, uint32_t i)
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
    return ES_STUB;
}

int es_offer(struct es_peer *a, uint32_t b, uint8_t *c, size_t d, size_t *e,
             uint32_t *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return ES_STUB;
}

int es_admit(struct es_peer *a, uint16_t b, uint32_t c, unsigned d, uint32_t e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return ES_STUB;
}

int es_ack(struct es_peer *a, const uint8_t *b, size_t c, uint32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return ES_STUB;
}

int es_receive(struct es_peer *a, const uint8_t *b, size_t c, uint32_t d,
               uint8_t *e, size_t f, size_t *g)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    return ES_STUB;
}

int es_deliver(struct es_peer *a, size_t b, struct es_event *c)
{
    (void)a;
    (void)b;
    (void)c;
    return ES_STUB;
}

int es_uncertain(struct es_peer *a, uint16_t b, uint16_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return ES_STUB;
}

int es_permitted(const struct es_peer *a)
{
    (void)a;
    return 0;
}
