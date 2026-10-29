#include "integration.h"

int in_records(const uint8_t *p, size_t n, algo_ring *q, size_t *used)
{
    if (!p || !q || !used)
        return -1;
    struct f_cursor c = {p, n, 0};
    *used = 0;
    while (c.position < n)
    {
        size_t start = c.position;
        const uint8_t *length, *text;
        int32_t value;
#ifdef IN_LOOKAHEAD
        struct f_cursor probe = c;
        if (f_take(&probe, 1, &length) != F_OK || !*length ||
            f_take(&probe, *length, &text) != F_OK ||
            f_parse_i32(text, *length, &value) != F_OK)
            return -1;
        if (algo_ring_put(q, value) != ALGO_OK)
            return 1;
        c = probe;
#else
        if (f_take(&c, 1, &length) != F_OK || !*length ||
            f_take(&c, *length, &text) != F_OK ||
            f_parse_i32(text, *length, &value) != F_OK)
            return -1;
        if (algo_ring_put(q, value) != ALGO_OK)
        {
            c.position = start;
            return 1;
        }
#endif
        (void)start;
        *used = c.position;
    }
    return 0;
}

int in_drain(algo_ring *q, struct in_transfer *t, f_writer_fn writer, void *ctx,
             unsigned budget)
{
    if (!q || !t || !writer)
        return -1;
    while (budget--)
    {
        if (!t->valid)
        {
            int32_t x;
            if (algo_ring_take(q, &x) != ALGO_OK)
                return 0;
            if (x < 0 || x > 255)
                return -1;
            t->pending = (uint8_t)x;
            t->valid = 1;
        }
        size_t used = 0;
        enum f_status r = f_write_all(writer, ctx, &t->pending, 1, 1, &used);
        if (used == 1)
            t->valid = 0;
        if (r != F_OK && r != F_PENDING)
            return -1;
        if (r != F_OK)
            return 1;
    }
    return t->valid || q->count ? 1 : 0;
}

int in_stream(struct in_stream *s, uint8_t byte, algo_ring *q)
{
    if (!s || !q || s->failed)
        return -1;
    if (!s->length)
    {
        if (!byte || byte > 31)
        {
            s->failed = 1;
            return -1;
        }
        s->needed = (size_t)byte + 1;
    }
    s->bytes[s->length++] = byte;
    if (s->length == s->needed)
    {
        size_t used;
        int r = in_records(s->bytes, s->length, q, &used);
        if (r)
        {
            s->failed = 1;
            return -1;
        }
        s->length = 0;
        s->needed = 0;
        return 1;
    }
    return 0;
}

int in_shutdown(algo_ring *q, struct f_async *op, unsigned completed,
                uint32_t ticket, unsigned *discarded)
{
    if (!q || !op || !discarded)
        return -1;
    if (op->state == F_INFLIGHT && f_cancel(op, ticket) != F_PENDING)
        return -1;
    if (op->state == F_CANCEL_REQUESTED)
    {
        if (!completed)
            return 1;
        if (f_complete(op, ticket) != F_OK)
            return -1;
    }
    if (op->state != F_DONE && op->state != F_IDLE)
        return -1;
    int32_t x;
    while (algo_ring_take(q, &x) == ALGO_OK)
        (*discarded)++;
    return 0;
}

int in_filter(struct v4_filter *f, const int32_t *input, size_t n, int32_t *out,
              size_t cap, const struct v4_sample *begin,
              const struct v4_sample *end, struct v4_measurement *m)
{
    size_t bytes;
    if (!f || !m || (!input && n) || (!out && n) || n > cap ||
        f_checked_size(n, sizeof *out, 0, &bytes) != F_OK)
        return -1;
    struct v4_measurement measured;
    if (v4_measure(begin, end, &measured))
        return -1;
    for (size_t i = 0; i < n; i++)
    {
        int32_t fir;
        if (v4_filter(f, input[i], &fir, &out[i]))
            return -1;
    }
    *m = measured;
    (void)bytes;
    return 0;
}
#ifdef IN_FIRMWARE
int in_dma_handoff(struct fw_dma *d, struct fw_io io, uint32_t ticket,
                   algo_ring *q)
{
    if (!d || !q || q->count == q->capacity || ticket > INT32_MAX)
        return -1;
    if (fw_dma(d, io, 2, ticket))
        return -1;
    return algo_ring_put(q, (int32_t)ticket) == ALGO_OK ? 0 : -1;
}

int in_gateway(struct fw_io io, algo_ring *q, struct v4_filter *f,
               struct fw_watchdog *w, int32_t *output)
{
    if (!q || !f || !w || !output || q->count == q->capacity)
        return -1;
    uint16_t sample;
    if (fw_adc(io, &sample, 4))
        return -1;
    if (algo_ring_put(q, sample) != ALGO_OK)
        return -1;
    int32_t value, fir;
    if (algo_ring_take(q, &value) != ALGO_OK ||
        v4_filter(f, value, &fir, output))
        return -1;
    return fw_watchdog(w, io, w->required, 0) == 1 ? 0 : -1;
}
#endif
#ifdef IN_VM_STORE
#include "cvm.h"
#include "store.h"

int in_vm_store(const char *source, size_t n, uint8_t *page, size_t cap)
{
    if (!page || cap < STORE_PAGE_SIZE)
        return -1;
    struct cvm_program p;
    struct cvm_cvm vm;
    struct cvm_value result;
    if (cvm_compile(source, n, &p) != CVM_OK)
        return -1;
    cvm_init(&vm, 1000, 32, 0);
    if (cvm_run(&vm, &p, &result) != CVM_OK || result.tag != CVM_INT)
        return -1;
    store_node node = {0};
    node.leaf = 1;
    node.n = 1;
    node.klen[0] = 1;
    node.key[0][0] = 'v';
    node.vlen[0] = 4;
    uint32_t bits = (uint32_t)result.integer;
    for (unsigned i = 0; i < 4; i++)
        node.value[0][i] = (uint8_t)(bits >> (8 * i));
    return store_page_encode(1, 1, &node, page) == STORE_OK ? 0 : -1;
}
#endif
