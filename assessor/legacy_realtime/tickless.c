#include "tickless.h"
#ifndef TQ_MUTANT
#define TQ_MUTANT 0
#endif
#define HALF UINT32_C(0x80000000)

enum tq_status tq_relation(uint32_t now, uint32_t at, unsigned bits,
                           unsigned infinite, enum tq_relation *out)
{
    if ((bits != 8 && bits != 16 && bits != 32) || infinite > 1)
    {
        return TQ_INVALID;
    }
    uint32_t mask = bits == 32 ? UINT32_MAX : (UINT32_C(1) << bits) - 1u;
    if ((now & mask) != now || (at & mask) != at)
    {
        return TQ_INVALID;
    }
    uint32_t distance = (at - now) & mask, half = UINT32_C(1) << (bits - 1);
    enum tq_relation r = infinite           ? TQ_INFINITE
                         : !distance        ? TQ_EQUAL
                         : distance == half ? TQ_AMBIGUOUS
                         : distance < half  ? TQ_FUTURE
                                            : TQ_PAST;
    if (TQ_MUTANT == 1 && r == TQ_AMBIGUOUS)
    {
        r = TQ_FUTURE;
    }
    *out = r;
    return TQ_OK;
}

/* Divisor<=10^12: the restoring remainder never needs a65th bit. */
static uint64_t divide(uint64_t n, uint64_t d, uint64_t *remainder)
{
    uint64_t q = 0, r = 0;
    for (unsigned bit = 64; bit;)
    {
        --bit;
        r = (r << 1) | ((n >> bit) & 1u);
        if (r >= d)
        {
            r -= d;
            q |= UINT64_C(1) << bit;
        }
    }
    *remainder = r;
    return q;
}

static unsigned product(uint64_t a, uint32_t b, uint64_t *out)
{
    uint64_t sum = 0;
    while (b)
    {
        if (b & 1u)
        {
            if (a > UINT64_MAX - sum)
            {
                return 0;
            }
            sum += a;
        }
        b >>= 1;
        if (b)
        {
            if (a > UINT64_MAX - a)
            {
                return 0;
            }
            a += a;
        }
    }
    *out = sum;
    return 1;
}

static enum tq_status convert(uint64_t value, uint32_t num, uint32_t den,
                              uint32_t scale, uint64_t *out)
{
    if (!num || !den || num > 1000000u || den > 1000000u)
    {
        return TQ_INVALID;
    }
    uint64_t divisor = (uint64_t)den * scale, rem,
             q = divide(value, divisor, &rem), base;
    if (!product(q, num, &base))
    {
        return TQ_FULL;
    }
    uint64_t fraction = rem * num, tail = divide(fraction, divisor, &rem);
    if (rem && TQ_MUTANT != 3)
    {
        ++tail;
    }
    if (base > UINT64_MAX - tail)
    {
        return TQ_FULL;
    }
    *out = base + tail;
    return TQ_OK;
}

enum tq_status tq_ticks(uint64_t ms, uint32_t num, uint32_t den, uint64_t *out)
{
    return convert(ms, num, den, 1000, out);
}

enum tq_status tq_not_before(uint64_t now, uint64_t ms, uint32_t num,
                             uint32_t den, uint64_t *out)
{
    uint64_t delay;
    if (!product(ms, 1000, &delay) || delay > UINT64_MAX - now)
    {
        return TQ_FULL;
    }
    if (TQ_MUTANT == 4)
    {
        uint64_t n, d;
        enum tq_status s = convert(now, num, den, 1000000, &n);
        if (s != TQ_OK)
        {
            return s;
        }
        s = tq_ticks(ms, num, den, &d);
        if (s != TQ_OK || n > UINT64_MAX - d)
        {
            return TQ_FULL;
        }
        *out = n + d - 1;
        return TQ_OK;
    }
    return convert(now + delay, num, den, 1000000, out);
}

enum tq_status tq_init(struct tq_timer *timer, uint32_t now)
{
    struct tq_timer t = {0};
    cmx_init(&t.kernel, CMX_FIXED);
    t.kernel.now = now;
    for (unsigned i = 0; i < TQ_TASKS; i++)
    {
        if (cmx_task_add(&t.kernel, i, 0, 0x1000 + 0x1000 * i, 512) != (int)i)
        {
            return TQ_INVALID;
        }
    }
    *timer = t;
    return TQ_OK;
}

enum tq_status tq_wait(struct tq_timer *t, unsigned task, uint32_t delay,
                       unsigned infinite)
{
    if (task >= TQ_TASKS || infinite > 1 || (!infinite && delay >= HALF) ||
        t->kernel.task[task].state != CMX_READY || (t->events & (1u << task)))
    {
        return TQ_INVALID;
    }
    if (t->kernel.sequence == UINT32_MAX || t->serial == UINT64_MAX)
    {
        return TQ_FULL;
    }
    enum cmx_result r =
        cmx_sleep(&t->kernel, task,
                  infinite ? cmx_forever() : cmx_at(t->kernel.now + delay));
    if (r != CMX_BLOCKED && r != CMX_TIMEOUT)
    {
        return TQ_INVALID;
    }
    if (r == CMX_TIMEOUT)
    {
        t->events |= 1u << task;
        t->dispatch[task] = t->kernel.now;
        t->kernel.task[task].result = CMX_OK;
    }
    t->armed = 0;
    return TQ_OK;
}

enum tq_status tq_cancel(struct tq_timer *t, unsigned task)
{
    if (task >= TQ_TASKS || t->kernel.task[task].state != CMX_WAITING)
    {
        return TQ_INVALID;
    }
    if (cmx_cancel(&t->kernel, task) != CMX_OK)
    {
        return TQ_INVALID;
    }
    t->events |= 1u << task;
    t->dispatch[task] = t->kernel.now;
    t->armed = 0;
    return TQ_OK;
}

enum tq_status tq_advance(struct tq_timer *t, uint32_t now)
{
    if ((uint32_t)(now - t->kernel.now) >= HALF)
    {
        return TQ_INVALID;
    }
    unsigned waiting = 0;
    for (unsigned i = 0; i < TQ_TASKS; i++)
    {
        if (t->kernel.task[i].state == CMX_WAITING)
        {
            waiting |= 1u << i;
        }
    }
    cmx_tick(&t->kernel, TQ_MUTANT == 2 ? now - 1 : now);
    t->kernel.now = now;
    for (unsigned i = 0; i < TQ_TASKS; i++)
    {
        if ((waiting & (1u << i)) && t->kernel.task[i].state == CMX_READY)
        {
            t->events |= 1u << i;
            t->dispatch[i] = now;
            t->armed = 0;
        }
    }
    return TQ_OK;
}

enum tq_status tq_collect(struct tq_timer *t, unsigned task, uint32_t *when,
                          enum cmx_result *result)
{
    if (task >= TQ_TASKS)
    {
        return TQ_INVALID;
    }
    if (!(t->events & (1u << task)))
    {
        return TQ_EMPTY;
    }
    *when = t->dispatch[task];
    *result = t->kernel.task[task].result;
    t->events &= ~(1u << task);
    return TQ_OK;
}

static struct tq_plan earliest(const struct tq_timer *t)
{
    struct tq_plan plan = {0};
    plan.observed = t->kernel.now;
    uint32_t distance = 0;
    for (unsigned i = 0; i < TQ_TASKS; i++)
    {
        if (t->kernel.task[i].state == CMX_WAITING &&
            !t->kernel.task[i].deadline.infinite)
        {
            uint32_t delta = t->kernel.task[i].deadline.at - t->kernel.now;
            if (!plan.armed || delta < distance)
            {
                plan.armed = 1;
                plan.at = t->kernel.task[i].deadline.at;
                distance = delta;
            }
        }
    }
    return plan;
}

enum tq_status tq_plan(struct tq_timer *t, uint32_t now, struct tq_plan *out)
{
    if (t->prepared)
    {
        return TQ_INVALID;
    }
    if (t->serial == UINT64_MAX)
    {
        return TQ_FULL;
    }
    if (tq_advance(t, now) != TQ_OK)
    {
        return TQ_INVALID;
    }
    struct tq_plan plan = earliest(t);
    plan.token = ++t->serial;
    t->plan = plan;
    t->prepared = 1;
    t->armed = 0;
    *out = plan;
    return TQ_OK;
}

enum tq_status tq_commit(struct tq_timer *t, struct tq_plan plan,
                         uint32_t after, unsigned irq)
{
    if (!t->prepared || irq > 1 || plan.token != t->plan.token ||
        plan.at != t->plan.at || plan.armed != t->plan.armed ||
        plan.observed != t->plan.observed ||
        (uint32_t)(after - t->kernel.now) >= HALF)
    {
        return TQ_INVALID;
    }
    if (tq_advance(t, after) != TQ_OK)
    {
        return TQ_INVALID;
    }
    struct tq_plan current = earliest(t);
    t->prepared = 0;
    t->armed = 0;
    if (irq || (TQ_MUTANT != 5 && (current.armed != plan.armed ||
                                   (current.armed && current.at != plan.at))))
    {
        return TQ_RETRY;
    }
    /* A newly ready task forbids sleep even when no finite timer remains. */
    if (t->events)
    {
        return TQ_RETRY;
    }
    t->armed = plan.armed;
    t->programmed = plan.at;
    return TQ_OK;
}

enum tq_status tq_abort(struct tq_timer *t)
{
    t->prepared = 0;
    t->armed = 0;
    return TQ_OK;
}

enum tq_status tq_resume(struct tq_timer *t, struct tq_resume *r, uint32_t now)
{
    if (t->prepared || !r->numerator || !r->denominator ||
        r->numerator > 1000000u || r->denominator > 1000000u ||
        r->remainder >= r->denominator)
    {
        return TQ_INVALID;
    }
    uint32_t delta = now - r->last;
    if (delta >= HALF)
    {
        return TQ_INVALID;
    }
    uint64_t remainder, total = (uint64_t)delta * r->numerator + r->remainder,
                        elapsed = divide(total, r->denominator, &remainder);
    if (elapsed >= HALF)
    {
        return TQ_FULL;
    }
    if (tq_advance(t, t->kernel.now + (uint32_t)elapsed) != TQ_OK)
    {
        return TQ_INVALID;
    }
    if (TQ_MUTANT != 6)
    {
        r->last = now;
    }
    r->remainder = remainder;
    t->armed = 0;
    return TQ_OK;
}
