/*
 * Reference implementation of the byte and span transfers declared in
 * writer.h. See that header for each function's contract.
 *
 * Compile-time mutants (author campaign only): CWF_MUTANT_REQUEUE re-queues an
 * accepted byte; CWF_MUTANT_SUFFIX forgets the accepted prefix on failure.
 */
#include "writer.h"

/*
 * File-local constants: BYTE_OPERATION is the diagnostic operation id of a
 * byte transfer; SPAN_STAGE_BYTES (bytes) bounds cwf_span_take; the cause code
 * 1001 is recorded as error.primary; WRITER_RETRY is the writer result meaning
 * "nothing accepted, try again", not a failure.
 */
enum writer_constant
{
    BYTE_OPERATION = 42,
    SPAN_STAGE_BYTES = 16,
    CAUSE_ADAPTER_FAILURE = 1001,
    WRITER_RETRY = -2
};

/* Lets the in_drain() callback update the byte transfer's diagnostic record. */
struct cwf_bridge
{
    struct cwf_byte *state;
    f_writer_fn writer;
    void *context;
};

/*
 * Forwards to the caller's writer and records accepted counts and the first
 * terminal cause. A negative result other than the retry code is terminal.
 */
static ptrdiff_t byte_write(void *context, const uint8_t *p, size_t n)
{
    struct cwf_bridge *b = context;
    const ptrdiff_t r = b->writer(b->context, p, n);
    ++b->state->calls;
    if (r > 0 && (size_t)r <= n)
    {
        b->state->error.accepted += (size_t)r;
    }
    if (r < 0 && r != WRITER_RETRY && b->state->error.primary == 0)
    {
        b->state->error.primary = CAUSE_ADAPTER_FAILURE;
    }
    return r;
}

int cwf_byte_step(algo_ring *q, struct cwf_byte *s, f_writer_fn write,
                  void *context, unsigned budget)
{
    if (q == NULL || s == NULL || write == NULL || s->failed)
    {
        return -1;
    }
    s->error.operation = BYTE_OPERATION;
#ifdef CWF_MUTANT_REQUEUE
    const size_t accepted_before = s->error.accepted;
#endif
    struct cwf_bridge bridge = {s, write, context};
    const int result = in_drain(q, &s->active, byte_write, &bridge, budget);
    if (result < 0)
    {
        s->failed = 1;
    }
#ifdef CWF_MUTANT_REQUEUE
    if (!s->requeued && s->error.accepted > accepted_before && !s->active.valid)
    {
        if (algo_ring_put(q, s->active.pending) != ALGO_OK)
        {
            return -1;
        }
        s->requeued = 1;
    }
#endif
    return result;
}

int cwf_byte_recover(struct cwf_byte *s)
{
    if (s == NULL || !s->failed)
    {
        return -1;
    }
    s->failed = 0;
    ++s->recoveries;
    return 0;
}

int cwf_span_begin(struct cwf_span *s, const unsigned char *p, size_t n,
                   unsigned operation)
{
    if (s == NULL || s->active || (p == NULL && n != 0) || n > sizeof s->bytes)
    {
        return -1;
    }
    for (size_t i = 0; i < n; ++i)
    {
        s->bytes[i] = p[i];
    }
    s->count = n;
    s->active = 1;
    const enum cpw_status status =
        cpw_begin(&s->transfer, s->bytes, n, operation);
    return status == CPW_INVALID ? -1 : 0;
}

/*
 * Copies n queue values into bytes[] without removing them. Returns 0 on
 * success, -1 if any value is not a byte. The candidate ring is a by-value
 * copy, which is valid because algo_ring holds indices and values only, with
 * no internal self-pointer.
 */
static int stage_prefix(algo_ring *candidate, unsigned char *bytes, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        int32_t value;
        if (algo_ring_take(candidate, &value) != ALGO_OK || value < 0 ||
            value > 255)
        {
            return -1;
        }
        bytes[i] = (unsigned char)value;
    }
    return 0;
}

int cwf_span_take(algo_ring *q, struct cwf_span *s, size_t n,
                  unsigned operation)
{
    unsigned char bytes[SPAN_STAGE_BYTES];
    if (q == NULL || s == NULL || s->active || n > sizeof bytes || n > q->count)
    {
        return -1;
    }
    /* Validate on a copy, then commit queue removal and the span together. */
    algo_ring candidate = *q;
    if (stage_prefix(&candidate, bytes, n) != 0)
    {
        return -1;
    }
    if (cwf_span_begin(s, bytes, n, operation) != 0)
    {
        return -1;
    }
    *q = candidate;
    return 0;
}

enum cpw_status cwf_span_step(struct cwf_span *s, cpw_write_fn write,
                              void *context)
{
    if (s == NULL || !s->active || write == NULL)
    {
        return CPW_INVALID;
    }
    const enum cpw_status status = cpw_step(&s->transfer, write, context);
#ifdef CWF_MUTANT_SUFFIX
    if (status == CPW_FAILED)
    {
        s->transfer.accepted = 0;
    }
#endif
    if (status == CPW_DONE)
    {
        s->active = 0;
    }
    return status;
}

enum cpw_status cwf_span_recover(struct cwf_span *s)
{
    if (s == NULL || !s->active)
    {
        return CPW_INVALID;
    }
    return cpw_recover(&s->transfer);
}

/*
 * Maps a transfer status to a diagnostic category: pending is CWF_WAIT, done is
 * CWF_COMPLETE, and every other status (failed or otherwise) is CWF_IO.
 * cwf_error() screens out statuses that are not pending, done or failed first.
 */
static enum cwf_category category_of(enum cpw_status status)
{
    switch (status)
    {
        case CPW_PENDING:
            return CWF_WAIT;
        case CPW_DONE:
            return CWF_COMPLETE;
        default:
            return CWF_IO;
    }
}

int cwf_error(const struct cpw_transfer *t, struct cwf_error *out)
{
    if (t == NULL || out == NULL ||
        (t->status != CPW_PENDING && t->status != CPW_DONE &&
         t->status != CPW_FAILED))
    {
        return -1;
    }
    out->category = category_of(t->status);
    out->operation = t->error.operation;
    out->accepted = t->error.accepted;
    out->primary = t->error.primary;
    out->cleanup = t->error.cleanup;
    return 0;
}
