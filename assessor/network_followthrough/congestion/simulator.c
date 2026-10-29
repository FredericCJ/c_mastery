/* Original finite byte-delivery model. No sockets, external targets or kernel
 * congestion controller. Receiver holds out-of-order blocks to produce real
 * cumulative ACK histories; network service and ACK travel are distinct. */
#include "congestion.h"
#include "ip.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum
{
    MSS = 64,
    BLOCKS = 128,
    EVENTS = 1024
};

struct nc_event
{
    unsigned used, flow, index, ack;
    uint64_t due;
    struct ni_packet packet;
};

struct nc_flow
{
    struct nr_control control;
    unsigned char received[BLOCKS][MSS], present[BLOCKS];
    unsigned next, delivered, sent, fast, timeouts, drops, wire, high_flight,
        at100, retransmitted[BLOCKS], ack_wait, ack_packets;
    uint32_t rto;
    uint64_t due, finished, ack_due;
};

struct nc_queued
{
    unsigned flow, index;
    struct ni_packet packet;
};
static struct nc_flow flows[2];
static struct nc_event events[EVENTS];
static struct nc_queued queue[64];
static unsigned head, count, capacity, mode, flow_count, asymmetric, order_bias;
static unsigned delayed_ack, limited_transmit;
static uint64_t now;

static void w16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v >> 8);
    p[1] = (unsigned char)v;
}

static void w32(unsigned char *p, uint32_t v)
{
    w16(p, v >> 16);
    w16(p + 2, v);
}

static unsigned u16(const unsigned char *p)
{
    return (unsigned)p[0] * 256 + p[1];
}

static uint32_t u32(const unsigned char *p)
{
    return (uint32_t)u16(p) << 16 | u16(p + 2);
}

static unsigned checksum(const unsigned char *p, size_t n)
{
    struct s_checksum s = {0, 0, 0};
    uint16_t out;
    (void)s_checksum_add(&s, p, n);
    (void)s_checksum_finish(&s, &out);
    return out;
}

static unsigned tcp_sum(const unsigned char *p, size_t n)
{
    unsigned char tail[4] = {0, 6, 0, (unsigned char)n};
    struct s_checksum s = {0, 0, 0};
    uint16_t out;
    (void)s_checksum_add(&s, p + 12, 8);
    (void)s_checksum_add(&s, tail, 4);
    (void)s_checksum_add(&s, p + 20, n);
    (void)s_checksum_finish(&s, &out);
    return out;
}

static unsigned char byte(unsigned flow, unsigned index, unsigned offset)
{
    return (unsigned char)(flow * 71 + index * 17 + offset);
}

static void packet(struct ni_packet *out, unsigned flow, unsigned index,
                   unsigned ack)
{
    memset(out, 0, sizeof *out);
    unsigned char *p = out->bytes;
    size_t n = ack ? 40 : 40 + MSS;
    out->length = n;
    p[0] = 0x45;
    w16(p + 2, (unsigned)n);
    w16(p + 6, 0x4000);
    p[8] = 64;
    p[9] = 6;
    w32(p + 12, 0xc0000201);
    w32(p + 16, 0xc0000202);
    w16(p + 20, 40000 + flow);
    w16(p + 22, 8080);
    w32(p + 24, 0xfffff000u + index * MSS);
    w32(p + 28, ack ? 0xfffff000u + index * MSS : 0);
    p[32] = 0x50;
    p[33] = ack ? 16 : 24;
    w16(p + 34, 16384);
    if (!ack)
        for (unsigned i = 0; i < MSS; i++)
            p[40 + i] = byte(flow, index, i);
    w16(p + 10, checksum(p, 20));
    w16(p + 36, tcp_sum(p, n - 20));
}

static int valid(const struct ni_packet *p)
{
    struct ni_config c = {0xc0000202, 0xc0000201};
    struct ni_view v;
    return ni_parse(&c, p->bytes, p->length, 0, &v) == NI_OK &&
           v.protocol == 6 && v.length >= 20 && v.payload[12] == 0x50 &&
           u16(v.payload + 2) == 8080 && !tcp_sum(p->bytes, v.length);
}

static int schedule(unsigned flow, unsigned index, unsigned ack, uint64_t due,
                    const struct ni_packet *p)
{
    for (unsigned i = 0; i < EVENTS; i++)
        if (!events[i].used)
        {
            events[i] = (struct nc_event){1, flow, index, ack, due, *p};
            return 0;
        }
    return 1;
}

static int enqueue(unsigned flow, unsigned index)
{
    struct nc_flow *f = &flows[flow];
    f->wire++;
    if (count == capacity)
    {
        f->drops++;
        return 0;
    }
    unsigned slot = (head + count) % capacity;
    queue[slot].flow = flow;
    queue[slot].index = index;
    packet(&queue[slot].packet, flow, index, 0);
    count++;
    return 0;
}

static int retransmit(unsigned flow)
{
    struct nc_flow *f = &flows[flow];
    unsigned index = (f->control.una - 0xfffff000u) / MSS;
    if (index >= BLOCKS)
        return 1;
    f->retransmitted[index]++;
    return enqueue(flow, index);
}

static int acknowledge(unsigned flow)
{
    struct nc_flow *f = &flows[flow];
    struct ni_packet ack;
    packet(&ack, flow, f->next, 1);
    f->ack_wait = 0;
    f->ack_due = 0;
    f->ack_packets++;
    return schedule(flow, f->next, 1, now + 5 + (asymmetric && flow ? 10 : 0),
                    &ack);
}

static int step(void)
{
    /* Receive events before timeout at equal time: explicit deterministic policy.
     */
    for (unsigned i = 0; i < EVENTS; i++)
        if (events[i].used && events[i].due <= now)
        {
            struct nc_event e = events[i];
            events[i].used = 0;
            if (!valid(&e.packet))
                return 10;
            struct nc_flow *f = &flows[e.flow];
            if (e.ack)
            {
                uint32_t ack = u32(e.packet.bytes + 28), old = f->control.una;
                int rc = nr_ack(&f->control, ack, 16384, 0, 16, now);
                if (rc == NR_INVALID)
                    return 11;
                if (rc == NR_FAST_RETRANSMIT)
                {
                    f->fast++;
                    if (retransmit(e.flow))
                        return 12;
                    f->due = now + f->rto;
                }
                if (f->control.una != old)
                    f->due = nr_flight(&f->control) ? now + f->rto : 0;
                if (f->control.una == 0xfffff000u + BLOCKS * MSS &&
                    !f->finished)
                    f->finished = now;
            }
            else
            {
                unsigned index = (u32(e.packet.bytes + 24) - 0xfffff000u) / MSS;
                if (index >= BLOCKS || e.packet.length != 40 + MSS)
                    return 13;
                for (unsigned k = 0; k < MSS; k++)
                    if (e.packet.bytes[40 + k] != byte(e.flow, index, k))
                        return 14;
                unsigned expected_index = f->next;
                memcpy(f->received[index], e.packet.bytes + 40, MSS);
                f->present[index] = 1;
                while (f->next < BLOCKS && f->present[f->next])
                {
                    f->next++;
                    f->delivered += MSS;
                }
                /* Gaps, duplicate segments and gap filling are ACKed immediately.
                 * Otherwise ACK at least every two full segments or after40ms. */
                if (!delayed_ack || index != expected_index ||
                    f->next > expected_index + 1 || ++f->ack_wait >= 2)
                {
                    if (acknowledge(e.flow))
                        return 15;
                }
                else if (!f->ack_due)
                    f->ack_due = now + 40;
            }
        }
    for (unsigned f = 0; f < flow_count; f++)
        if (flows[f].ack_due && now >= flows[f].ack_due)
            if (acknowledge(f))
                return 29;
    for (unsigned j = 0; j < flow_count; j++)
    {
        unsigned flow = order_bias == 2 ? flow_count - 1 - j
                        : order_bias
                            ? j
                            : (j + (unsigned)(now % flow_count)) % flow_count;
        struct nc_flow *f = &flows[flow];
        if (nr_flight(&f->control) && f->due && now >= f->due)
        {
            if (nr_timeout(&f->control, now) != NR_OK)
                return 16;
            f->timeouts++;
            f->rto = f->rto > 30000 ? 60000 : f->rto * 2;
            f->due = now + f->rto;
            if (retransmit(flow))
                return 17;
        }
        while (f->sent < BLOCKS && nr_usable(&f->control) >= MSS)
        {
            uint32_t seq;
            unsigned was_empty = !nr_flight(&f->control);
            if (nr_send(&f->control, MSS, 1, now, &seq) != NR_OK ||
                seq != 0xfffff000u + f->sent * MSS)
                return 18;
            if (was_empty)
                f->due = now + f->rto;
            if (enqueue(flow, f->sent++))
                return 19;
            if (nr_flight(&f->control) > f->high_flight)
                f->high_flight = nr_flight(&f->control);
        }
    }
    /* One packet per tick shared bottleneck; accepted NIC copies can be dropped
     * later by this finite network queue. That is not a lower-copy refusal. */
    if (count)
    {
        struct nc_queued q = queue[head];
        head = (head + 1) % capacity;
        count--;
        struct nc_flow *f = &flows[q.flow];
        unsigned discard =
            (mode == 1 && q.index == 8 && !f->retransmitted[8]) ||
            (mode == 2 && (q.index == 8 || q.index == 10) &&
             !f->retransmitted[q.index]);
        if (discard)
            f->drops++;
        else
        {
            uint64_t delay = 5 + (asymmetric && q.flow ? 10 : 0);
            if (mode == 3 && q.index == 8 && !f->retransmitted[8])
                delay += 40;
            if (schedule(q.flow, q.index, 0, now + delay, &q.packet))
                return 20;
        }
    }
    if (now == 100)
        for (unsigned f = 0; f < flow_count; f++)
            flows[f].at100 = flows[f].delivered;
    return 0;
}

static int run(unsigned case_id, unsigned selected, unsigned cap,
               unsigned nflow, unsigned asym, unsigned bias, unsigned delayed,
               unsigned limited)
{
    memset(flows, 0, sizeof flows);
    memset(events, 0, sizeof events);
    head = count = 0;
    mode = selected;
    capacity = cap;
    flow_count = nflow;
    asymmetric = asym;
    order_bias = bias;
    delayed_ack = delayed;
    limited_transmit = limited;
    for (unsigned f = 0; f < nflow; f++)
    {
        if (nr_init(&flows[f].control, MSS, 0xfffff000u, 16384) != NR_OK)
            return 21;
        if (nr_policy(&flows[f].control, limited_transmit) != NR_OK)
            return 30;
        flows[f].rto = 1000;
    }
    for (now = 0; now < 200000; now++)
    {
        int rc = step();
        if (rc)
        {
            printf("NR_SIM_ERROR case=%u code=%d tick=%llu\n", case_id, rc,
                   (unsigned long long)now);
            return rc;
        }
        unsigned done = 1;
        for (unsigned f = 0; f < nflow; f++)
            done &= flows[f].finished != 0;
        if (done && now >= 100)
            break;
    }
    if (now == 200000)
        return 22;
    for (unsigned f = 0; f < nflow; f++)
    {
        struct nc_flow *s = &flows[f];
        if (s->delivered != BLOCKS * MSS || nr_flight(&s->control))
            return 23;
        for (unsigned i = 0; i < BLOCKS; i++)
            for (unsigned k = 0; k < MSS; k++)
                if (s->received[i][k] != byte(f, i, k))
                    return 24;
        if (case_id == 0 && (s->drops || s->fast || s->timeouts))
            return 25;
        if (case_id == 1 && (!s->fast || s->timeouts || s->drops != 1))
            return 26;
        if (case_id == 2 && (!s->fast || !s->timeouts || s->drops != 2))
            return 27;
        if (case_id == 3 && (!s->fast || s->drops))
            return 28;
        printf(
            "NR_SIM case=%u flow=%u delivered=%u at100=%u finish=%llu fast=%u "
            "rto=%u drops=%u packets=%u maxflight=%u acks=%u\n",
            case_id, f, s->delivered, s->at100, (unsigned long long)s->finished,
            s->fast, s->timeouts, s->drops, s->wire, s->high_flight,
            s->ack_packets);
    }
    return 0;
}

int main(void)
{
    printf("NR_COST state=%zu flows=%zu events=%zu queue=%zu event_slots=%u\n",
           sizeof(struct nr_control), sizeof flows, sizeof events, sizeof queue,
           EVENTS);
    for (unsigned m = 0; m < 4; m++)
    {
        int rc = run(m, m, 64, 1, 0, 0, 0, 0);
        if (rc)
        {
            printf("NR_SIM_FAIL %u %d\n", m, rc);
            return 1;
        }
    }
    for (unsigned asym = 0; asym < 2; asym++)
        for (unsigned bias = 0; bias < 2; bias++)
        {
            int rc = run(4 + asym * 2 + bias, 0, 4, 2, asym, bias, 0, 0);
            if (rc)
            {
                printf("NR_SIM_FAIL %u %d\n", 4 + asym * 2 + bias, rc);
                return 1;
            }
        }
    if (run(8, 0, 4, 2, 0, 2, 0, 0) || run(9, 0, 64, 1, 0, 0, 1, 0) ||
        run(10, 1, 64, 1, 0, 0, 0, 1) || run(11, 2, 64, 1, 0, 0, 1, 1))
        return 1;
    puts("NR_SIM_PASS twelve_schedules_actual_IP_checksum_exact_bytes");
    return 0;
}
