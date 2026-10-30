/*
 * Freestanding strict-image target: replays the byte-transfer trace through
 * the reference writer with no libc. cpw_image() is called by the entry
 * adapter and its return value becomes the process exit status.
 *
 * Trace: 65 is retried (-2), 65 is accepted, 66 hits a terminal error (-1),
 * explicit recovery resumes, and 66 is accepted. Each failing step returns
 * its own code so the host can tell which transition broke.
 */
#include "writer.h"

enum target_failure
{
    TARGET_OK = 0,
    TARGET_SETUP = 81,
    TARGET_RETRY = 82,
    TARGET_FIRST_ACCEPT = 83,
    TARGET_TERMINAL = 84,
    TARGET_RECOVERY = 85,
    TARGET_FINAL_STATE = 86
};

enum target_value
{
    SINK_RETRY = -2,
    SINK_ERROR = -1,
    BYTE_A = 65,
    BYTE_B = 66,
    CAUSE_ADAPTER_FAILURE = 1001
};

/* Sink that scripts call 0 as retry and call 2 as a terminal error. */
struct cwf_sink
{
    unsigned call;
    unsigned used;
    unsigned char bytes[2];
};

static ptrdiff_t write_byte(void *context, const uint8_t *p, size_t n)
{
    struct cwf_sink *s = context;
    if (n != 1)
    {
        return SINK_ERROR;
    }
    const unsigned call = s->call++;
    if (call == 0)
    {
        return SINK_RETRY;
    }
    if (call == 2)
    {
        return SINK_ERROR;
    }
    s->bytes[s->used++] = *p;
    return 1;
}

int cpw_image(void)
{
    algo_ring q;
    struct cwf_byte t = {0};
    struct cwf_sink sink = {0};
    if (algo_ring_init(&q, 2) != ALGO_OK ||
        algo_ring_put(&q, BYTE_A) != ALGO_OK ||
        algo_ring_put(&q, BYTE_B) != ALGO_OK)
    {
        return TARGET_SETUP;
    }
    if (cwf_byte_step(&q, &t, write_byte, &sink, 1) != 1 || !t.active.valid)
    {
        return TARGET_RETRY;
    }
    if (cwf_byte_step(&q, &t, write_byte, &sink, 1) < 0 || t.active.valid)
    {
        return TARGET_FIRST_ACCEPT;
    }
    if (cwf_byte_step(&q, &t, write_byte, &sink, 1) != -1 || !t.failed ||
        t.active.pending != BYTE_B)
    {
        return TARGET_TERMINAL;
    }
    if (cwf_byte_recover(&t) || cwf_byte_step(&q, &t, write_byte, &sink, 1) < 0)
    {
        return TARGET_RECOVERY;
    }
    if (q.count || t.active.valid || sink.used != 2 ||
        sink.bytes[0] != BYTE_A || sink.bytes[1] != BYTE_B ||
        t.error.primary != CAUSE_ADAPTER_FAILURE)
    {
        return TARGET_FINAL_STATE;
    }
    return TARGET_OK;
}
