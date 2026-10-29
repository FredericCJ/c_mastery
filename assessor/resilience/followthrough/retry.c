#include "retry.h"

int rzr_reduce(uint32_t sample, unsigned bits, uint32_t span, uint32_t *out)
{
    if (!out || !bits || bits > 32 || !span)
    {
        return RZ_INVALID;
    }
    uint64_t range = UINT64_C(1) << bits;
    if (sample >= range || span > range)
    {
        return RZ_INVALID;
    }
    uint32_t threshold = (uint32_t)(range % span);
#if RZF_MUTANT != 41
    if (sample < threshold)
    {
        return RZ_PENDING;
    }
#else
    (void)threshold;
#endif
    *out = sample % span;
    return RZ_OK;
}

int rzr_uniform(struct rzr_random *random, uint32_t span, uint32_t *out)
{
    if (!random || !out || !span)
    {
        return RZ_INVALID;
    }
    for (unsigned attempt = 0; attempt < 64; ++attempt)
    {
        if (random->draws == UINT64_MAX)
        {
            return RZ_FULL;
        }
        random->state = random->state * UINT32_C(5) + UINT32_C(1);
        ++random->draws;
        uint32_t sample = random->state;
        sample ^= sample >> 16;
        sample *= UINT32_C(0x9e3779b1);
        sample ^= sample >> 13;
        int status = rzr_reduce(sample, 32, span, out);
        if (status == RZ_OK)
        {
            return status;
        }
    }
    return RZ_PENDING;
}

static bool live(const struct rzr_retry *r)
{
    return r && !r->closed && r->scope.scope;
}

int rzr_init(struct rzr_retry *out, struct rzf_scope *scope,
             struct rzf_bucket *limiter, struct net_stack *transport,
             unsigned retries, uint64_t base, uint64_t cap, bool safe,
             uint32_t seed)
{
    if (!out || !scope || !limiter || !transport || !transport->flight ||
        transport->state != NET_ESTABLISHED || !transport->retained.length ||
        transport->retained.length > sizeof transport->retained.bytes ||
        transport->retries || !transport->rto || transport->rto > 60000 ||
        retries > 3)
    {
        return RZ_INVALID;
    }
    struct rzr_retry r = {0};
    int status =
        rz_retry_init(&r.retry, &scope->core, retries, base, cap, safe);
    if (status != RZ_OK)
    {
        return status;
    }
    status = rzf_borrow_begin(&r.scope, scope);
    if (status != RZ_OK)
    {
        return status;
    }
    r.limiter = limiter;
    r.transport = transport;
    r.random.state = seed;
    *out = r;
    return RZ_OK;
}

static int clock_check(const struct rzr_retry *r, uint64_t now)
{
    if (now < r->transport->now || now > UINT64_MAX - 240000)
    {
        return RZ_RANGE;
    }
    return rz_check(&r->scope.scope->core, now);
}

int rzr_original(struct rzr_retry *r, uint64_t now, v4_packet_sink_fn sink,
                 void *context)
{
    if (!live(r) || !sink)
    {
        return RZ_INVALID;
    }
    if (r->original)
    {
        return RZ_DONE;
    }
    int status = clock_check(r, now);
    if (status != RZ_OK)
    {
        return status;
    }
    status = rzf_bucket_consume(r->limiter, now, 1);
    if (status != RZ_OK)
    {
        return status;
    }
    r->transport->now = r->transport->sent_at = now;
    r->transport->due = now + r->transport->rto;
    r->original = 1;
    ++r->dispatched;
    int lost = sink(context, &r->transport->retained);
    if (lost)
    {
        ++r->dropped;
    }
    return lost ? RZ_PENDING : RZ_OK;
}

int rzr_plan(struct rzr_retry *r, uint64_t now, uint64_t retry_after,
             enum rzr_jitter policy, uint64_t *out)
{
    if (!live(r) || !out || policy < RZR_FULL_JITTER ||
        policy > RZR_EQUAL_JITTER)
    {
        return RZ_INVALID;
    }
    if (!r->original)
    {
        return RZ_INVALID;
    }
    if (r->pending)
    {
        return RZ_BUSY;
    }
    if (!r->transport->flight)
    {
        return RZ_DONE;
    }
    int status = clock_check(r, now);
    if (status != RZ_OK)
    {
        return status;
    }
    if (!r->retry.idempotent)
    {
        return RZ_DENIED;
    }
    if (r->retry.attempts == r->retry.maximum)
    {
        return RZ_FULL;
    }
    if (retry_after > UINT64_MAX - now)
    {
        return RZ_RANGE;
    }
    uint64_t window = r->retry.base;
    for (unsigned i = 0; i < r->retry.attempts; ++i)
    {
        window = window > r->retry.cap / 2 ? r->retry.cap : window * 2;
    }
    uint32_t draw = (uint32_t)window;
    if (policy != RZR_NO_JITTER)
    {
        uint32_t lower =
            policy == RZR_EQUAL_JITTER ? (uint32_t)(window / 2) : 0;
        status = rzr_uniform(&r->random, (uint32_t)window - lower + 1, &draw);
        if (status != RZ_OK)
        {
            return status;
        }
        draw += lower;
    }
    struct rz_retry staged = r->retry;
    uint64_t due;
    status = rz_retry_next(&staged, now, draw, &due);
    if (status != RZ_OK)
    {
        return status;
    }
    if (due < now + retry_after)
    {
        due = now + retry_after;
    }
    if (due < r->transport->due)
    {
        due = r->transport->due;
    }
    status = rz_check(&r->scope.scope->core, due);
    if (status != RZ_OK)
    {
        return status;
    }
    r->retry = staged;
    r->due = due;
    r->pending = true;
    *out = due;
    return RZ_OK;
}

int rzr_dispatch(struct rzr_retry *r, uint64_t now, v4_packet_sink_fn sink,
                 void *context)
{
    if (!live(r) || !sink)
    {
        return RZ_INVALID;
    }
    if (!r->pending)
    {
        return RZ_EMPTY;
    }
    if (!r->transport->flight)
    {
        r->pending = false;
        return RZ_DONE;
    }
    int status = clock_check(r, now);
    if (status == RZ_CANCELLED || status == RZ_EXPIRED)
    {
        r->pending = false;
        return status;
    }
    if (status != RZ_OK)
    {
        return status;
    }
    if (now < r->due)
    {
        return RZ_PENDING;
    }
#if RZF_MUTANT != 42
    status = rzf_bucket_consume(r->limiter, now, 1);
    if (status != RZ_OK)
    {
        return status;
    }
#endif
    r->pending = false;
    ++r->dispatched;
    int result = v4_tcp_delivery(r->transport, now, sink, context, &r->dropped);
    return result < 0 ? RZ_CLOSED : result ? RZ_OK : RZ_PENDING;
}

int rzr_ack_packet(struct rzr_retry *r, const void *bytes, size_t length,
                   uint64_t now, struct net_packet *out)
{
    if (!live(r) || !out || (!bytes && length))
    {
        return RZ_INVALID;
    }
    if (now < r->transport->now || now > UINT64_MAX - 240000)
    {
        return RZ_RANGE;
    }
    /* Cancellation does not suppress the independent transport acknowledgement. */
    int result = net_receive(r->transport, bytes, length, now, out);
    if (!r->transport->flight)
    {
        r->pending = false;
    }
    return result < 0 ? RZ_INVALID : RZ_OK;
}

int rzr_finish(struct rzr_retry *r)
{
    if (!live(r))
    {
        return RZ_INVALID;
    }
    if (r->transport->flight || r->pending)
    {
        return RZ_BUSY;
    }
    int status = rzf_borrow_ack(&r->scope);
    if (status == RZ_OK)
    {
        r->closed = true;
    }
    return status;
}

int rzr_abort(struct rzr_retry *r)
{
    if (!live(r))
    {
        return RZ_INVALID;
    }
    net_link_down(r->transport);
    r->pending = false;
    return rzr_finish(r);
}
