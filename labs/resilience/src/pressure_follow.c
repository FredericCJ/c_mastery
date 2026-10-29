#include "pressure.h"

void rzp_owned_init(struct rzp_owned_queue *p)
{
    if (p)
    {
        *p = (struct rzp_owned_queue){0};
    }
}

void rzp_value_drop(struct rzp_value *p)
{
    (void)p;
}

int rzp_owned_offer(struct rzp_owned_queue *p, const struct rz_scope *s,
                    uint64_t n, enum rz_policy x, struct rzp_value *v)
{
    (void)p;
    (void)s;
    (void)n;
    (void)x;
    (void)v;
    return RZ_INVALID;
}

int rzp_owned_pop(struct rzp_owned_queue *p, struct rzp_value *v)
{
    (void)p;
    (void)v;
    return RZ_INVALID;
}

void rzp_owned_close(struct rzp_owned_queue *p)
{
    (void)p;
}

int rzp_init(struct rzp_pump *p, size_t n)
{
    (void)p;
    (void)n;
    return RZ_INVALID;
}

int rzp_begin(struct rzp_pump *p, unsigned i, struct rzf_scope *s, uint64_t n,
              struct rzp_value *v, struct rzp_ticket *t)
{
    (void)p;
    (void)i;
    (void)s;
    (void)n;
    (void)v;
    (void)t;
    return RZ_INVALID;
}

int rzp_poll(struct rzp_pump *p, struct rzp_ticket t, bool w, int *v)
{
    (void)p;
    (void)t;
    (void)w;
    (void)v;
    return RZ_INVALID;
}

int rzp_cancel(struct rzp_pump *p, struct rzp_ticket t)
{
    (void)p;
    (void)t;
    return RZ_INVALID;
}

int rzp_tick(struct rzp_pump *p, uint64_t n)
{
    (void)p;
    (void)n;
    return RZ_INVALID;
}

int rzp_forward(struct rzp_pump *p)
{
    (void)p;
    return RZ_INVALID;
}

int rzp_receive(struct rzp_pump *p, struct rzp_value *v, struct rzp_credit *c)
{
    (void)p;
    (void)v;
    (void)c;
    return RZ_INVALID;
}

int rzp_release(struct rzp_pump *p, struct rzp_credit c)
{
    (void)p;
    (void)c;
    return RZ_INVALID;
}

int rzp_close(struct rzp_pump *p)
{
    (void)p;
    return RZ_INVALID;
}

int rzp_destroy(struct rzp_pump *p)
{
    (void)p;
    return RZ_INVALID;
}

int rzp_snapshot(struct rzp_pump *p, struct rzp_snapshot *s)
{
    (void)p;
    (void)s;
    return RZ_INVALID;
}
