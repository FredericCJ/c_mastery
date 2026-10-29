#include "v4_com.h"

int v4_reassembly(struct v4_reassembly *s, const struct edu_fragment *f,
                  struct edu_fragment *ack, uint64_t now, unsigned grant)
{
    if (!s || !f || !ack || now < s->last || now > UINT64_MAX - 100)
    {
        return -1;
    }
    s->last = now;
    if (s->rx.active && now >= s->due)
    {
        s->rx.active = 0;
        s->rx.length = 0;
        s->rx.index = 0;
    }
    s->granted = grant;
    if (!grant)
    {
        return -1;
    }
    unsigned was_active = s->rx.active;
    int rc = edu_rx_accept(&s->rx, f, ack);
    if (rc >= 0 && s->rx.active && !was_active)
    {
        s->due = now + 100;
    }
    return rc;
}

int v4_arp_queue(struct v4_arp *a, uint32_t peer, const uint8_t *p, size_t n,
                 uint64_t now)
{
    if (!a || !p || !n || n > 64 || a->pending)
    {
        return -1;
    }
    a->peer = peer;
    a->length = n;
    for (size_t i = 0; i < n; i++)
    {
        a->bytes[i] = p[i];
    }
    a->due = now;
    a->pending = 1;
    a->retries = 0;
    a->resolved = 0;
    return 0;
}

int v4_arp_poll(struct v4_arp *a, uint64_t now)
{
    if (!a || !a->pending)
    {
        return 0;
    }
    if (a->resolved)
    {
        a->pending = 0;
        return 2;
    }
    if (now < a->due)
    {
        return 0;
    }
    if (a->retries == 3 || now > UINT64_MAX - 100)
    {
        a->pending = 0;
        return -1;
    }
    a->retries++;
    a->due = now + 100;
    return 1;
}

int v4_arp_resolve(struct v4_arp *a, uint32_t peer)
{
    if (!a || !a->pending || peer != a->peer)
    {
        return -1;
    }
    a->resolved = 1;
    return 0;
}

int v4_tcp_delivery(struct net_stack *s, uint64_t now, v4_packet_sink_fn sink,
                    void *ctx, unsigned *drops)
{
    if (!s || !sink || !drops)
    {
        return -1;
    }
    struct net_packet p;
    int r = net_tick(s, now, &p);
    if (r < 0)
    {
        return -1;
    }
    if (p.length && sink(ctx, &p))
    {
        if (*drops == UINT32_MAX)
        {
            return -1;
        }
        (*drops)++;
        return 0;
    }
    return p.length ? 1 : 0;
}

static int wait_reg(struct v4_can_io io, unsigned reg, uint32_t mask,
                    uint32_t want, unsigned polls)
{
    while (polls--)
    {
        if ((io.read(io.ctx, reg) & mask) == want)
        {
            return 0;
        }
    }
    return -1;
}

int v4_can_init(struct v4_can_io io, uint32_t nominal, uint32_t data,
                unsigned polls)
{
    if (!io.read || !io.write || !nominal || !data || !polls)
    {
        return -1;
    }
    io.write(io.ctx, 0, 1);
    if (wait_reg(io, 0, 1, 1, polls))
    {
        return -1;
    }
    io.write(io.ctx, 0, 3);
    io.write(io.ctx, 1, nominal);
    io.write(io.ctx, 2, data);
    io.write(io.ctx, 3, 1);
    io.write(io.ctx, 0, 0);
    return wait_reg(io, 0, 1, 0, polls);
}

int v4_can_loopback(struct v4_can_io io, const uint8_t *p, size_t n,
                    uint8_t *out, size_t cap, unsigned polls)
{
    if (!io.read || !io.write || !p || !out || n > 64 || !n || cap < n ||
        !(n <= 8 || n == 12 || n == 16 || n == 20 || n == 24 || n == 32 ||
          n == 48 || n == 64))
    {
        return -1;
    }
    for (size_t i = 0; i < n; i++)
    {
        io.write(io.ctx, 16 + (unsigned)i, p[i]);
    }
    io.write(io.ctx, 4, (uint32_t)n);
    if (wait_reg(io, 5, 1, 1, polls))
    {
        return -1;
    }
    for (size_t i = 0; i < n; i++)
    {
        out[i] = (uint8_t)io.read(io.ctx, 16 + (unsigned)i);
    }
    io.write(io.ctx, 5, 0);
    return 0;
}
