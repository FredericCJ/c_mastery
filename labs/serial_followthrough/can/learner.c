#include "can.h"

int cn_feed(struct cn_stream *a, const uint8_t *b, size_t c, size_t *d,
            struct cn_decoded *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return CN_UNIMPLEMENTED;
}

int cn_search(uint32_t a, uint32_t b, unsigned c, unsigned d, uint32_t e,
              struct cn_search *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return CN_UNIMPLEMENTED;
}

int cn_fault(struct cn_fault *a, const struct cn_event *b, struct cn_effect *c)
{
    (void)a;
    (void)b;
    (void)c;
    return CN_UNIMPLEMENTED;
}

int cn_begin(struct cn_controller *a, struct cn_io b, const struct sd_timing *c,
             unsigned d, unsigned e, unsigned f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return CN_UNIMPLEMENTED;
}

int cn_control(struct cn_controller *a)
{
    (void)a;
    return CN_UNIMPLEMENTED;
}

int cn_recover(struct cn_controller *a, unsigned b)
{
    (void)a;
    (void)b;
    return CN_UNIMPLEMENTED;
}

int cn_send(struct cn_controller *a, const struct sd_can *b, uint32_t *c)
{
    (void)a;
    (void)b;
    (void)c;
    return CN_UNIMPLEMENTED;
}

int cn_tx_poll(struct cn_controller *a, uint32_t b)
{
    (void)a;
    (void)b;
    return CN_UNIMPLEMENTED;
}

int cn_cancel(struct cn_controller *a, uint32_t b)
{
    (void)a;
    (void)b;
    return CN_UNIMPLEMENTED;
}

int cn_take(struct cn_controller *a, uint32_t b, unsigned *c)
{
    (void)a;
    (void)b;
    (void)c;
    return CN_UNIMPLEMENTED;
}

int cn_receive(struct cn_controller *a, struct cn_rx *b)
{
    (void)a;
    (void)b;
    return CN_UNIMPLEMENTED;
}

int cn_ack_overrun(struct cn_controller *a)
{
    (void)a;
    return CN_UNIMPLEMENTED;
}

int cn_queue_put(struct cn_queue *a, const struct cn_rx *b)
{
    (void)a;
    (void)b;
    return CN_UNIMPLEMENTED;
}

int cn_queue_get(struct cn_queue *a, struct cn_rx *b)
{
    (void)a;
    (void)b;
    return CN_UNIMPLEMENTED;
}

struct cn_io cn_f439_io(void)
{
    struct cn_io io = {0};
    return io;
}
