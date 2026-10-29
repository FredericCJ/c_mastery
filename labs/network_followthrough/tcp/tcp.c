#include "tcp.h"

/* Deliberately incomplete learner implementation. */
int nc_rtt_init(struct nc_rtt *a0, unsigned a1, unsigned a2)
{
    (void)a0;
    (void)a1;
    (void)a2;
    return NC_STUB;
}

int nc_rtt_sample(struct nc_rtt *a0, unsigned a1, unsigned a2)
{
    (void)a0;
    (void)a1;
    (void)a2;
    return NC_STUB;
}

int nc_init(struct nc_stack *a0, struct ni_config a1, uint32_t a2, uint32_t a3,
            unsigned a4, unsigned a5)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    return NC_STUB;
}

int nc_receive(struct nc_stack *a0, const unsigned char *a1, size_t a2,
               uint64_t a3)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    return NC_STUB;
}

int nc_tick(struct nc_stack *a0, uint64_t a1)
{
    (void)a0;
    (void)a1;
    return NC_STUB;
}

int nc_offer(struct nc_stack *a0, const void *a1, size_t a2)
{
    (void)a0;
    (void)a1;
    (void)a2;
    return NC_STUB;
}

int nc_close(struct nc_stack *a0)
{
    (void)a0;
    return NC_STUB;
}

int nc_abort(struct nc_stack *a0, uint64_t a1)
{
    (void)a0;
    (void)a1;
    return NC_STUB;
}

int nc_prepare(struct nc_stack *a0, struct nc_packet *a1)
{
    (void)a0;
    (void)a1;
    return NC_STUB;
}

int nc_submit(struct nc_stack *a0, struct nc_ticket a1, unsigned a2,
              uint64_t a3)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    return NC_STUB;
}

int nc_dma_done(struct nc_stack *a0, struct nc_ticket a1)
{
    (void)a0;
    (void)a1;
    return NC_STUB;
}

int nc_consume(struct nc_stack *a0, size_t a1)
{
    (void)a0;
    (void)a1;
    return NC_STUB;
}

int nc_http_pump(struct nc_stack *a0, struct http_parser *a1, size_t a2,
                 size_t *a3)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    return NC_STUB;
}

int nc_http_eof(struct nc_stack *a0, struct http_parser *a1)
{
    (void)a0;
    (void)a1;
    return NC_STUB;
}

unsigned nc_usable(const struct nc_stack *a0)
{
    (void)a0;
    return NC_STUB;
}
