/*
 * Public tests for the byte and span transfers in writer.h.
 *
 *   tests         byte transfer: retry, terminal failure, recovery, resume
 *   tests span    copied span: staged take, partial writes, failure, recovery
 *
 * Exit status 0 prints CWF_BYTE_PASS or CWF_SPAN_PASS; a failed requirement
 * prints "CWF_ASSERT:<expression>" and exits 41.
 */
#include "writer.h"
#include <stdio.h>
#include <string.h>
#define REQUIRE(x)                                  \
    do                                              \
    {                                               \
        if (!(x))                                   \
        {                                           \
            fprintf(stderr, "CWF_ASSERT:%s\n", #x); \
            return 41;                              \
        }                                           \
    } while (0)

enum test_limit
{
    SINK_CALLS = 12,
    SINK_OUTPUT_BYTES = 32,
    SPAN_BYTES = 9,
    SPAN_OPERATION = 71,
    SPAN_RING_CAPACITY = 16,
    BYTE_RING_CAPACITY = 4
};

/*
 * Scripted sink. results[next] is the value the next callback returns
 * (negative errno, -2 retry, or an accepted count); the sink records what it
 * was offered so tests can check offsets and remaining lengths per call.
 */
struct cwf_sink
{
    long results[SINK_CALLS];
    size_t next;
    size_t used;
    size_t offset[SINK_CALLS];
    size_t remaining[SINK_CALLS];
    unsigned char output[SINK_OUTPUT_BYTES];
    const unsigned char *base;
};

static ptrdiff_t byte_write(void *context, const uint8_t *p, size_t n)
{
    struct cwf_sink *s = context;
    const long r = s->results[s->next++];
    if (r > 0 && (size_t)r <= n)
    {
        for (long i = 0; i < r; ++i)
        {
            s->output[s->used++] = p[i];
        }
    }
    return (ptrdiff_t)r;
}

static struct cpw_result span_write(void *context, const unsigned char *p,
                                    size_t n)
{
    struct cwf_sink *s = context;
    const size_t call = s->next;
    s->offset[call] = (size_t)(p - s->base);
    s->remaining[call] = n;
    const struct cpw_result out = cpw_raw(s->results[s->next++]);
    if (out.kind == CPW_PROGRESS && out.accepted <= n)
    {
        for (size_t j = 0; j < out.accepted; ++j)
        {
            s->output[s->used++] = p[j];
        }
    }
    return out;
}

/*
 * Span trace for "ABCDEFGHI" with results 3, EIO (-5), 2, 4: the failure after
 * 3 bytes keeps offset 3, and recovery delivers the other 6 exactly once.
 */
static int check_span_transfer(void)
{
    struct cwf_span s = {0};
    algo_ring q;
    unsigned char original[] = "ABCDEFGHI";
    struct cwf_sink sink = {{3, -5, 2, 4}, 0, 0, {0}, {0}, {0}, s.bytes};
    struct cwf_error error;
    REQUIRE(algo_ring_init(&q, SPAN_RING_CAPACITY) == ALGO_OK);
    for (unsigned i = 0; i < SPAN_BYTES; ++i)
    {
        REQUIRE(algo_ring_put(&q, original[i]) == ALGO_OK);
    }
    /* Asking for more than the ring holds must leave the ring untouched. */
    REQUIRE(cwf_span_take(&q, &s, 10, SPAN_OPERATION) == -1 && q.count == 9 &&
            !s.active);
    REQUIRE(cwf_span_take(&q, &s, 9, SPAN_OPERATION) == 0 && !q.count);
    /* The span owns a copy: changing the source must not change the output. */
    memset(original, 'X', SPAN_BYTES);
    REQUIRE(cwf_span_step(&s, span_write, &sink) == CPW_PENDING &&
            s.transfer.accepted == 3 && s.active);
    REQUIRE(cwf_span_step(&s, span_write, &sink) == CPW_FAILED &&
            s.transfer.accepted == 3 && sink.used == 3);
    REQUIRE(cwf_span_step(&s, span_write, &sink) == CPW_FAILED &&
            sink.next == 2);
    cpw_cleanup(&s.transfer, 9);
    REQUIRE(s.transfer.error.primary == 5 && s.transfer.error.cleanup == 9);
    REQUIRE(cwf_error(&s.transfer, &error) == 0 && error.category == CWF_IO &&
            error.operation == 71 && error.accepted == 3 &&
            error.primary == 5 && error.cleanup == 9);
    REQUIRE(cwf_span_recover(&s) == CPW_PENDING);
    REQUIRE(cwf_span_step(&s, span_write, &sink) == CPW_PENDING &&
            s.transfer.accepted == 5);
    REQUIRE(cwf_span_step(&s, span_write, &sink) == CPW_DONE && !s.active);
    REQUIRE(sink.used == 9 && !memcmp(sink.output, "ABCDEFGHI", 9));
    REQUIRE(sink.offset[0] == 0 && sink.offset[1] == 3 && sink.offset[2] == 3 &&
            sink.offset[3] == 5 && sink.remaining[1] == 6);
    puts("CWF_SPAN_PASS");
    return 0;
}

/*
 * Byte trace: 65 retry (-2), 65 accepted, 66 terminal error, explicit
 * recovery, 66 accepted. The ring, the active byte and the sink must agree at
 * every step.
 */
static int check_byte_transfer(void)
{
    algo_ring q;
    struct cwf_byte s = {0};
    struct cwf_sink sink = {{-2, 1, -1, 1, 1}, 0, 0, {0}, {0}, {0}, NULL};
    REQUIRE(algo_ring_init(&q, BYTE_RING_CAPACITY) == ALGO_OK &&
            algo_ring_put(&q, 65) == ALGO_OK &&
            algo_ring_put(&q, 66) == ALGO_OK);
    REQUIRE(cwf_byte_step(&q, &s, byte_write, &sink, 1) == 1 &&
            s.active.valid && s.active.pending == 65 && q.count == 1 &&
            sink.used == 0);
    REQUIRE(cwf_byte_step(&q, &s, byte_write, &sink, 1) >= 0 &&
            !s.active.valid && sink.used == 1 && sink.output[0] == 65);
    REQUIRE(cwf_byte_step(&q, &s, byte_write, &sink, 1) == -1 && s.failed &&
            s.active.valid && s.active.pending == 66 && sink.used == 1);
    REQUIRE(cwf_byte_step(&q, &s, byte_write, &sink, 1) == -1 &&
            sink.next == 3);
    REQUIRE(cwf_byte_recover(&s) == 0);
    REQUIRE(cwf_byte_step(&q, &s, byte_write, &sink, 1) >= 0 &&
            sink.used == 2 && sink.output[1] == 66);
    /* One more step drains the ring; its result is checked through state. */
    (void)cwf_byte_step(&q, &s, byte_write, &sink, 1);
    printf("CWF_SINK:%zu", sink.used);
    for (size_t i = 0; i < sink.used; ++i)
    {
        printf(":%u", sink.output[i]);
    }
    putchar('\n');
    REQUIRE(sink.used == 2 && !q.count && !s.active.valid &&
            s.error.accepted == 2 && s.error.primary == 1001 &&
            s.recoveries == 1);
    puts("CWF_BYTE_PASS");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "span") == 0)
    {
        return check_span_transfer();
    }
    return check_byte_transfer();
}
