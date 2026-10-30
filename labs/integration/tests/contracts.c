/*
 * Contract tests for the integration handoffs: `contracts CASE` runs one
 * scenario (p1..p6, plus p4/mcu with IN_FIRMWARE and linux with IN_VM_STORE).
 *
 * Exit status: 0 pass; 1 a failed CHECK (printed as "line: expression");
 * 2 unknown case name or wrong argument count.
 */
#include "integration.h"
#include <stdio.h>
#include <string.h>
#ifdef IN_VM_STORE
#include "store.h"
#endif
#define CHECK(x)                                       \
    do                                                 \
    {                                                  \
        if (!(x))                                      \
        {                                              \
            fprintf(stderr, "%d: %s\n", __LINE__, #x); \
            return 1;                                  \
        }                                              \
    } while (0)

enum contract_limit
{
    RING_CAPACITY = 4,
    SINK_CAPACITY = 16
};

/* Scripted byte sink: `stall` makes the next call report "retry". */
struct in_sink
{
    uint8_t out[SINK_CAPACITY];
    unsigned n;
    unsigned calls;
    unsigned stall;
};

static ptrdiff_t writer(void *p, const uint8_t *b, size_t n)
{
    struct in_sink *s = p;
    s->calls++;
    if (s->stall)
    {
        s->stall = 0;
        return -2;
    }
    if (!n || s->n == SINK_CAPACITY)
    {
        return -1;
    }
    s->out[s->n++] = *b;
    return 1;
}

static ptrdiff_t failed_writer(void *p, const uint8_t *b, size_t n)
{
    (void)p;
    (void)b;
    (void)n;
    return -1;
}
#ifdef IN_FIRMWARE
/* Register-file model of the firmware I/O seam; counts watchdog feeds. */
struct in_model
{
    uint32_t r[16];
    unsigned feeds;
};

static uint32_t rd(void *p, unsigned r)
{
    return ((struct in_model *)p)->r[r];
}

static void wr(void *p, unsigned r, uint32_t v)
{
    struct in_model *m = p;
    m->r[r] = v;
    if (r == FW_WATCHDOG)
    {
        m->feeds++;
    }
}
#endif

/* P1: complete records commit in order; a malformed one publishes nothing. */
static int case_records(algo_ring *q)
{
    uint8_t p[] = {2, '1', '2', 2, '-', '3'};
    size_t used = 99;
    CHECK(in_records(p, sizeof p, q, &used) == 0 && used == sizeof p);
    int32_t v;
    CHECK(algo_ring_take(q, &v) == 0 && v == 12);
    CHECK(algo_ring_take(q, &v) == 0 && v == -3);
    uint8_t bad[] = {1, '7', 2, 'x', 'y'};
    CHECK(in_records(bad, sizeof bad, q, &used) < 0 && used == 2 &&
          q->count == 1);
    CHECK(algo_ring_take(q, &v) == 0 && v == 7);
    CHECK(algo_ring_init(q, 1) == 0);
    CHECK(in_records(p, sizeof p, q, &used) == 1 && used == 3 && q->count == 1);
    return 0;
}

/* P2: the active byte survives a stall and a terminal failure. */
static int case_drain(algo_ring *q)
{
    CHECK(algo_ring_put(q, 65) == 0 && algo_ring_put(q, 66) == 0);
    struct in_sink s = {.stall = 1};
    struct in_transfer t = {0};
    CHECK(in_drain(q, &t, writer, &s, 4) == 1 && t.valid && s.n == 0 &&
          q->count == 1);
    CHECK(in_drain(q, &t, writer, &s, 4) == 0 && s.n == 2 && s.out[0] == 65 &&
          s.out[1] == 66 && !t.valid && !q->count);
    CHECK(algo_ring_put(q, 67) == ALGO_OK);
    CHECK(in_drain(q, &t, failed_writer, &s, 4) < 0 && t.valid &&
          t.pending == 67 && !q->count && s.n == 2);
    return 0;
}

/* P3: bytewise input, 31-byte ceiling and a terminal malformed state. */
static int case_stream(algo_ring *q)
{
    struct in_stream s = {0};
    CHECK(in_stream(&s, 2, q) == 0 && !q->count);
    CHECK(in_stream(&s, '4', q) == 0 && !q->count);
    CHECK(in_stream(&s, '2', q) == 1 && q->count == 1);
    int32_t x;
    CHECK(algo_ring_take(q, &x) == 0 && x == 42);
    CHECK(in_stream(&s, 32, q) < 0 && s.failed && !q->count);
    CHECK(in_stream(&s, 1, q) < 0);
    return 0;
}

/* P5: cancellation is pending until completion is observed. */
static int case_shutdown(algo_ring *q)
{
    struct f_async op = {0};
    uint32_t ticket;
    CHECK(f_submit(&op, &ticket) == F_OK);
    CHECK(algo_ring_put(q, 1) == 0 && algo_ring_put(q, 2) == 0);
    unsigned discarded = 0;
    CHECK(in_shutdown(q, &op, 0, ticket, &discarded) == 1 && q->count == 2 &&
          !discarded);
    CHECK(in_shutdown(q, &op, 1, ticket, &discarded) == 0 && q->count == 0 &&
          discarded == 2 && op.releases == 1);
    CHECK(in_shutdown(q, &op, 1, ticket, &discarded) == 0 && discarded == 2 &&
          op.releases == 1);
    return 0;
}

/* P6: filter history survives chunk boundaries; output extent is checked. */
static int case_filter(algo_ring *q)
{
    (void)q;
    int32_t input[] = {4, 0, 0, 0};
    int32_t out[4];
    struct v4_filter f = {0};
    struct v4_sample a = {0};
    struct v4_sample b = {.wall_ns = 100, .cycles = 20};
    struct v4_measurement m;
    CHECK(in_filter(&f, input, 2, out, 2, &a, &b, &m) == 0);
    CHECK(in_filter(&f, input + 2, 2, out + 2, 2, &a, &b, &m) == 0);
    CHECK(out[0] == 0 && out[1] == 1 && out[2] == 1 && out[3] == 0 &&
          m.delta.cycles == 20);
    CHECK(in_filter(&f, input, 4, out, 3, &a, &b, &m) < 0);
    return 0;
}
#ifdef IN_FIRMWARE
/* P4: a DMA ticket is queued only once the device accepts the transfer. */
static int case_dma(algo_ring *q)
{
    struct in_model m = {0};
    struct fw_io io = {&m, rd, wr};
    struct fw_dma d = {0};
    CHECK(fw_dma(&d, io, 0, 0) == 0);
    CHECK(in_dma_handoff(&d, io, 1, q) < 0 && !q->count && d.state == 1);
    m.r[FW_DMA] = 0;
    CHECK(in_dma_handoff(&d, io, 1, q) == 0 && q->count == 1 && d.state == 0);
    CHECK(in_dma_handoff(&d, io, 1, q) < 0 && q->count == 1);
    return 0;
}

/* MCU: a failed acquisition publishes nothing and feeds no watchdog. */
static int case_gateway(algo_ring *q)
{
    struct in_model m = {0};
    struct fw_io io = {&m, rd, wr};
    struct fw_watchdog w = {.required = 3};
    struct v4_filter f = {0};
    int32_t out = 99;
    CHECK(in_gateway(io, q, &f, &w, &out) < 0 && !q->count && !m.feeds &&
          out == 99);
    m.r[FW_STATUS] = FW_EOC;
    m.r[FW_ADC] = 32;
    CHECK(in_gateway(io, q, &f, &w, &out) == 0 && out == 4 && !q->count &&
          m.feeds == 1);
    return 0;
}
#endif
#ifdef IN_VM_STORE
/* Linux: a VM result becomes a checksummed page; corruption is detected. */
static int case_vm_store(algo_ring *q)
{
    (void)q;
    uint8_t page[STORE_PAGE_SIZE];
    const char source[] = "(fn main () (* 6 7))";
    CHECK(in_vm_store(source, sizeof source - 1, page, sizeof page) == 0);
    store_node node;
    CHECK(store_page_decode(1, 1, page, &node) == STORE_OK && node.n == 1 &&
          node.value[0][0] == 42 && node.vlen[0] == 4);
    uint8_t old[STORE_PAGE_SIZE];
    memcpy(old, page, sizeof old);
    CHECK(in_vm_store("6 *", 3, page, sizeof page) < 0 &&
          !memcmp(old, page, sizeof old));
    page[100] ^= 1;
    CHECK(store_page_decode(1, 1, page, &node) != STORE_OK);
    return 0;
}
#endif

struct contract_case
{
    const char *name;
    int (*run)(algo_ring *);
};

static const struct contract_case cases[] = {
    {"p1", case_records},     {"p2", case_drain},    {"p3", case_stream},
    {"p5", case_shutdown},    {"p6", case_filter},
#ifdef IN_FIRMWARE
    {"p4", case_dma},         {"mcu", case_gateway},
#endif
#ifdef IN_VM_STORE
    {"linux", case_vm_store},
#endif
};

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    algo_ring q;
    CHECK(algo_ring_init(&q, RING_CAPACITY) == ALGO_OK);
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            return cases[i].run(&q);
        }
    }
    return 2;
}
