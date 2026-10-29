#include "retry.h"

int sr_timer_begin(struct sr_timer *a0, unsigned a1, uint32_t a2, uint32_t a3)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    return SR_UNIMPLEMENTED;
}

int sr_timer_step(struct sr_timer *a0, uint32_t a1, uint32_t *a2)
{
    (void)a0;
    (void)a1;
    (void)a2;
    return SR_UNIMPLEMENTED;
}

int sr_byte_begin(struct sr_byte *a0, unsigned a1, unsigned a2, uint32_t a3,
                  uint32_t a4, uint32_t a5)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    return SR_UNIMPLEMENTED;
}

int sr_byte_sample(struct sr_byte *a0, uint32_t a1, unsigned a2, unsigned a3,
                   unsigned a4)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    return SR_UNIMPLEMENTED;
}

int sr_txn_begin(struct sr_txn *a0, uint32_t a1, uint8_t a2, uint8_t a3,
                 unsigned a4, uint32_t a5, uint32_t a6)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    return SR_UNIMPLEMENTED;
}

int sr_txn_bit(struct sr_txn *a0, uint32_t a1, uint32_t a2, unsigned a3,
               unsigned a4)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    return SR_UNIMPLEMENTED;
}

int sr_txn_read(struct sr_txn *a0, uint32_t a1, uint32_t a2, uint8_t a3,
                uint8_t *a4)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    return SR_UNIMPLEMENTED;
}

int sr_retry_begin(struct sr_retry *a0, unsigned a1, uint32_t a2, uint32_t a3,
                   uint32_t a4, unsigned a5, unsigned a6, uint32_t a7,
                   uint32_t a8)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    (void)a8;
    return SR_UNIMPLEMENTED;
}

int sr_retry_poll(struct sr_retry *a0, uint32_t a1, unsigned a2, unsigned a3,
                  unsigned a4)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    return SR_UNIMPLEMENTED;
}

int sr_retry_complete(struct sr_retry *a0, uint32_t a1, uint32_t a2,
                      unsigned a3, unsigned a4)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    return SR_UNIMPLEMENTED;
}
