#include "composition.h"

struct rzc_state;

struct rz_attempt
{
    struct rzc_state *state;
    unsigned index, calls, due;
    struct net_stack transport;
    struct rzr_retry retry;
    struct rzh_ticket ticket;
    struct rzh_blob response;
    struct rzt_id timer;
    bool active, pending, scheduled, finished;
};

struct rzc_state
{
    struct rzc_config config;
    struct rzc_result result;
    uint64_t now;
    struct rzf_allocator allocator;
    struct rzf_scope *scope;
    struct rzf_bucket bucket;
    struct rzt_timers timers;
    struct rzh_arbiter hedge;
    struct rzj_journal journal;
    struct rzs_outbox outbox;
    struct msg_hub hub;
    struct rz_attempt attempts[2];
    bool reset;
};

static uint64_t clock_read(void *p)
{
    return ((struct rzc_state *)p)->now;
}

static int decode(void *p, const unsigned char *bytes, size_t n, unsigned *out)
{
    (void)p;
    if (n != RZV_FRAME_BYTES || !bytes)
    {
        return RZ_INVALID;
    }
    struct rzv_reply reply;
    int status = rzv_reply_decode(bytes, &reply);
    if (status != RZ_OK || reply.status || reply.result < 0)
    {
        return RZ_INVALID;
    }
    *out = (unsigned)reply.result;
    return RZ_OK;
}

static void dispose(void *p, unsigned char *bytes)
{
    struct rzc_state *s = p;
    ++s->result.freed_responses;
    s->allocator.deallocate(s->allocator.context, bytes);
}

static void put16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v >> 8);
    p[1] = (unsigned char)v;
}

static void put32(unsigned char *p, uint32_t v)
{
    put16(p, v >> 16);
    put16(p + 2, v);
}

static unsigned checksum(const unsigned char *p, size_t n, uint32_t sum)
{
    for (size_t i = 0; i < n; ++i)
    {
        sum += (uint32_t)p[i] << (i % 2 ? 0 : 8);
    }
    while (sum > 65535)
    {
        sum = (sum & 65535) + (sum >> 16);
    }
    return 65535 - sum;
}

static void ack_packet(struct net_packet *p, const struct net_stack *net)
{
    *p = (struct net_packet){0};
    const unsigned char localmac[6] = {2, 0, 0, 0, 0, 2},
                        peermac[6] = {2, 0, 0, 0, 0, 1};
    const unsigned char peerip[4] = {192, 0, 2, 1}, localip[4] = {192, 0, 2, 2};
    for (unsigned i = 0; i < 6; ++i)
    {
        p->bytes[i] = localmac[i];
        p->bytes[i + 6] = peermac[i];
    }
    put16(p->bytes + 12, 0x800);
    unsigned char *ip = p->bytes + 14;
    ip[0] = 0x45;
    put16(ip + 2, 40);
    put16(ip + 6, 0x4000);
    ip[8] = 64;
    ip[9] = 6;
    for (unsigned i = 0; i < 4; ++i)
    {
        ip[12 + i] = peerip[i];
        ip[16 + i] = localip[i];
    }
    put16(ip + 10, checksum(ip, 20, 0));
    unsigned char *tcp = ip + 20;
    put16(tcp, 40000);
    put16(tcp + 2, 8080);
    put32(tcp + 4, net->rcv_nxt);
    put32(tcp + 8, net->snd_nxt);
    tcp[12] = 0x50;
    tcp[13] = 16;
    put16(tcp + 14, 512);
    put16(tcp + 16,
          checksum(tcp, 20, 0xc000 + 0x0201 + 0xc000 + 0x0202 + 6 + 20));
    p->length = 60;
}

static int sink(void *p, const struct net_packet *packet)
{
    struct rz_attempt *a = p;
    struct rzc_state *s = a->state;
    unsigned call = a->calls++;
    if (s->config.drops[a->index] & (1u << call))
    {
        return 1;
    }
    struct rzv_frame frame = {0};
    struct rzj_request request;
    size_t used = 0;
    if (rzv_frame_feed(&frame, packet->bytes, packet->length, &used,
                       &request) != RZ_OK ||
        used != packet->length)
    {
        return 1;
    }
    int value = 0;
    bool duplicate = false;
    int status =
        rzs_apply(&s->outbox, s->now / 1000, request, &value, &duplicate);
    if (status == RZ_OK)
    {
        s->result.applied += !duplicate;
        s->result.replays += duplicate;
    }
    if (!a->pending)
    {
        a->pending = true;
        a->due = (unsigned)(s->now / 1000) + s->config.delay[a->index];
        unsigned char *bytes =
            s->allocator.allocate(s->allocator.context, RZV_FRAME_BYTES);
        if (bytes)
        {
            ++s->result.allocated_responses;
            (void)rzv_reply_encode(
                (struct rzv_reply){(unsigned)status,
                                   status == RZ_OK && duplicate,
                                   status == RZ_OK ? value : 0, 1},
                bytes);
            a->response = (struct rzh_blob){bytes, RZV_FRAME_BYTES, s, dispose};
        }
    }
    return 0;
}

static int schedule(struct rz_attempt *a)
{
    struct rzc_state *s = a->state;
    uint64_t due;
    int status = rzr_plan(&a->retry, s->now, s->config.retry_after,
                          RZR_FULL_JITTER, &due);
    if (status != RZ_OK)
    {
        return status;
    }
    uint64_t tick = due / 1000 + (due % 1000 != 0), current = s->now / 1000;
#if RZF_MUTANT == 111
    tick = due / 1000;
#endif
    if (tick <= current)
    {
        tick = current + 1;
    }
    status = rzt_schedule(&s->timers, current, tick - current, &a->timer);
    if (status == RZ_OK)
    {
        a->scheduled = true;
    }
    return status;
}

static int begin_attempt(struct rzc_state *s, unsigned index)
{
    struct rz_attempt *a = &s->attempts[index];
    a->state = s;
    a->index = index;
    if (index)
    {
        int check = rz_check(&s->scope->core, s->now);
        if (check != RZ_OK)
        {
            return check;
        }
        if (s->hedge.core.winner >= 0)
        {
            return RZ_DONE;
        }
    }
    struct rzf_bucket proposed = s->bucket;
    int status = rzf_bucket_consume(&proposed, s->now, 1);
    if (status != RZ_OK)
    {
        ++s->result.denied;
        return status;
    }
    if (index)
    {
        status = rzh_start(&s->hedge, &a->ticket);
        if (status != RZ_OK)
        {
            return status;
        }
    }
    net_init(&a->transport, 100);
    struct net_stack *net = &a->transport;
    net->state = NET_ESTABLISHED;
    net->flight = 1;
    net->peer_known = 1;
    net->peer_port = 40000;
    net->peer_window = 512;
    net->arp_expires = 60000;
    net->snd_una = 100;
    net->snd_nxt = 118;
    net->rcv_nxt = 1000;
    net->rto = 1000;
    net->now = s->now;
    net->due = s->now + 1000;
    net->retained.length = RZV_FRAME_BYTES;
    status = rzv_request_encode((struct rzj_request){0, 1, 1, 3},
                                net->retained.bytes);
    if (status != RZ_OK)
    {
        return status;
    }
    status = rzr_init(&a->retry, s->scope, &s->bucket, net, 3, 2, 8, true,
                      75009 + index);
    if (status != RZ_OK)
    {
        return status;
    }
    a->active = true;
    ++s->result.started;
    status = rzr_original(&a->retry, s->now, sink, a);
    if (status != RZ_OK && status != RZ_PENDING)
    {
        return status;
    }
    (void)schedule(a);
    return RZ_OK;
}

static int complete(struct rz_attempt *a, bool aborting)
{
    struct rzc_state *s = a->state;
    int status;
    if (a->scheduled)
    {
        (void)rzt_cancel(&s->timers, a->timer);
        a->scheduled = false;
    }
    if (aborting)
    {
        status = rzr_abort(&a->retry);
    }
    else
    {
        struct net_packet packet, out;
        ack_packet(&packet, &a->transport);
        status = rzr_ack_packet(&a->retry, packet.bytes, packet.length, s->now,
                                &out);
        if (status == RZ_OK)
        {
            status = rzr_finish(&a->retry);
        }
    }
    if (status != RZ_OK)
    {
        return status;
    }
    status = rzh_complete(&s->hedge, a->ticket, &a->response);
    a->finished = true;
    return status == RZ_OK || status == RZ_DONE || status == RZ_EXPIRED ||
                   status == RZ_CANCELLED
               ? RZ_OK
               : status;
}

int rzc_run(struct rzc_config config, struct rzj_io journal_io,
            struct rzj_io ack_io, struct rzf_allocator allocator,
            struct rzc_result *out)
{
    if (!out || !allocator.allocate || !allocator.deallocate ||
        !config.deadline || config.deadline > 16 || config.notify_at > 32 ||
        config.cancel_at > 16 || !config.capacity || config.capacity > 8 ||
        config.retry_after > 1501 || config.drops[0] > 15 ||
        config.drops[1] > 15 || !config.delay[0] || config.delay[0] > 24 ||
        !config.delay[1] || config.delay[1] > 24)
    {
        return RZ_INVALID;
    }
    struct rzc_state s = {
        .config = config, .allocator = allocator, .result = {.winner = -1}};
    int status = rzf_scope_create(NULL, 0, (uint64_t)config.deadline * 1000,
                                  allocator, &s.scope);
    if (status != RZ_OK)
    {
        return status;
    }
    status = rzj_open(journal_io, 16, &s.journal);
    if (status != RZ_OK || s.journal.records)
    {
        rzf_scope_release(&s.scope);
        return status == RZ_OK ? RZ_INVALID : status;
    }
    uint64_t epoch;
    status = rzj_issue(&s.journal, 0, &epoch);
    if (status != RZ_OK)
    {
        rzf_scope_release(&s.scope);
        return status;
    }
    if (msg_hub_init(&s.hub, 1, 1) != MSG_OK)
    {
        rzf_scope_release(&s.scope);
        return RZ_FULL;
    }
    status = rzs_open(&s.journal, ack_io, &s.hub, &s.outbox);
    if (status != RZ_OK)
    {
        msg_hub_destroy(&s.hub);
        rzf_scope_release(&s.scope);
        return status;
    }
    status = rzh_init(&s.hedge, &s, clock_read, decode);
    if (status != RZ_OK)
    {
        msg_hub_destroy(&s.hub);
        rzf_scope_release(&s.scope);
        return status;
    }
    (void)rzf_bucket_init(&s.bucket, config.capacity, 1, 65535, 0);
    (void)rzt_init(&s.timers, RZT_WHEEL, 0);
    status = rzh_begin(&s.hedge, s.scope, 2000, 0, 1, 1, &s.attempts[0].ticket);
    if (status != RZ_OK)
    {
        (void)rzh_destroy(&s.hedge);
        msg_hub_destroy(&s.hub);
        rzf_scope_release(&s.scope);
        return status;
    }
    status = begin_attempt(&s, 0);
    if (status != RZ_OK)
    {
        goto cleanup;
    }
    struct rzt_id deadline_timer = {0}, hedge_timer = {0};
    status = rzt_schedule(&s.timers, 0, config.deadline, &deadline_timer);
    if (status != RZ_OK)
    {
        goto cleanup;
    }
    if (config.hedge)
    {
        (void)rzt_schedule(&s.timers, 0, 2, &hedge_timer);
    }
    bool took = false, quiet = false, delivered_waiting = false,
         hedge_considered = !config.hedge, deadline_seen = false;
    for (unsigned tick = 0; tick <= 64; ++tick)
    {
        s.now = (uint64_t)tick * 1000;
        if (config.cancel_at && tick == config.cancel_at)
        {
            rz_cancel(&s.scope->core);
        }
        for (unsigned i = 0; i < 2; ++i)
        {
            struct rz_attempt *a = &s.attempts[i];
            if (!a->active || a->finished)
            {
                continue;
            }
            if (a->pending && a->due == tick)
            {
                status = complete(a, false);
                if (status != RZ_OK)
                {
                    goto cleanup;
                }
            }
            else if (!a->pending && deadline_seen)
            {
                status = complete(a, true);
                if (status != RZ_OK)
                {
                    goto cleanup;
                }
            }
        }
        struct rzh_result result = {0};
        if (!took && rzh_take(&s.hedge, &result) == RZ_OK)
        {
            took = true;
            s.result.winner = (int)result.attempt;
            s.result.reply_at = tick;
            if (result.blob.data)
            {
                result.blob.dispose(result.blob.context, result.blob.data);
            }
            if (!hedge_considered)
            {
                (void)rzt_cancel(&s.timers, hedge_timer);
                hedge_considered = true;
            }
        }
        struct rzt_events events;
        status = rzt_advance(&s.timers, tick, &events);
        if (status != RZ_OK)
        {
            goto cleanup;
        }
        for (unsigned k = 0; k < events.count; ++k)
        {
            struct rzt_id id = events.events[k].id;
            if (id.slot == deadline_timer.slot &&
                id.generation == deadline_timer.generation)
            {
                deadline_seen = true;
                continue;
            }
            if (config.hedge && id.slot == hedge_timer.slot &&
                id.generation == hedge_timer.generation)
            {
                hedge_considered = true;
                (void)begin_attempt(&s, 1);
                continue;
            }
            for (unsigned i = 0; i < 2; ++i)
            {
                struct rz_attempt *a = &s.attempts[i];
                if (!a->scheduled || a->timer.slot != id.slot ||
                    a->timer.generation != id.generation)
                {
                    continue;
                }
                a->scheduled = false;
                bool cancelled = false;
                (void)rzh_cancelled(&s.hedge, a->ticket, &cancelled);
                if (cancelled)
                {
                    continue;
                }
                status = rzr_dispatch(&a->retry, s.now, sink, a);
                if (status == RZ_DENIED)
                {
                    ++s.result.denied;
                    continue;
                }
                if (status == RZ_OK || status == RZ_PENDING)
                {
                    (void)schedule(a);
                }
            }
        }
        if (!quiet && hedge_considered && rzh_quiescent(&s.hedge) == RZ_OK)
        {
            quiet = true;
            s.result.quiet_at = tick;
            (void)rzt_cancel(&s.timers, deadline_timer);
        }
        if (s.outbox.pending.count)
        {
            (void)rzs_forward(&s.outbox);
        }
        if (tick >= config.notify_at && !delivered_waiting)
        {
            struct rzs_event event;
            if (rzs_deliver(&s.outbox, &event) == RZ_OK)
            {
                ++s.result.deliveries;
                delivered_waiting = true;
                s.result.notification_at = tick;
            }
        }
        if (delivered_waiting && (!config.reset_before_ack || s.reset))
        {
            status = rzs_ack(&s.outbox, s.outbox.events[0].sequence);
            if (status != RZ_OK)
            {
                goto cleanup;
            }
            ++s.result.acknowledged;
            delivered_waiting = false;
        }
        else if (delivered_waiting && config.reset_before_ack && !s.reset &&
                 quiet)
        {
            /* The delivered channel is already empty. Reconstruct only the
               quiescent service state; no pthread object is byte-copied/reset. */
            status = rzj_open(journal_io, 16, &s.journal);
            if (status != RZ_OK)
            {
                goto cleanup;
            }
            status = rzs_open(&s.journal, ack_io, &s.hub, &s.outbox);
            if (status != RZ_OK)
            {
                goto cleanup;
            }
#if RZF_MUTANT == 112
            s.outbox.pending.count = 0;
#endif
            s.reset = true;
            ++s.result.resets;
            delivered_waiting = false;
        }
        if (quiet && !s.outbox.count && tick >= config.notify_at)
        {
            break;
        }
    }
    if (s.journal.poisoned || s.outbox.poisoned)
    {
        status = RZJ_UNCERTAIN;
        goto cleanup;
    }
    if (!quiet || s.outbox.count)
    {
        status = RZ_BUSY;
        goto cleanup;
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        if (s.attempts[i].active)
        {
            s.result.dispatched += s.attempts[i].retry.dispatched;
            s.result.retries += s.attempts[i].retry.dispatched - 1;
            s.result.dropped += s.attempts[i].retry.dropped;
        }
    }
    s.result.total = s.journal.receiver.total;
    s.result.tokens = s.bucket.tokens;
    s.result.journal_records = s.journal.records;
    s.result.ack_records = s.outbox.ack_records;
    s.result.timer_work = s.timers.work;
    status = RZ_OK;
cleanup:
    (void)rzh_abandon(&s.hedge);
    for (unsigned i = 0; i < 2; ++i)
    {
        struct rz_attempt *a = &s.attempts[i];
        if (a->active && !a->retry.closed)
        {
            (void)rzr_abort(&a->retry);
        }
        if ((s.hedge.core.started & (1u << i)) &&
            !(s.hedge.finalized & (1u << i)))
        {
            (void)rzh_complete(&s.hedge, a->ticket, &a->response);
        }
    }
    {
        int cleaned = rzh_destroy(&s.hedge);
        if (status == RZ_OK)
        {
            status = cleaned;
        }
    }
    msg_hub_destroy(&s.hub);
    rzf_scope_release(&s.scope);
    if (status == RZ_OK)
    {
        *out = s.result;
    }
    return status;
}
