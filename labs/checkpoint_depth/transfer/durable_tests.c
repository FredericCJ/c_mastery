#include "durable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
    do                                                                         \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            fprintf(stderr, "CPX_CHECK: %s:%d: %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

struct cpx_device
{
    unsigned char working[STORE_IMAGE_PAGES][STORE_PAGE_SIZE],
        stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
    unsigned operation, cut, persist_before_error;
    struct cpx_sink *owner;
};
static struct cpx_device disk, snapshot;

static int read_page(void *ctx, uint32_t page, void *out)
{
    struct cpx_device *d = ctx;
    if (page >= STORE_IMAGE_PAGES)
        return 1;
    memcpy(out, d->working[page], STORE_PAGE_SIZE);
    return 0;
}

static int write_page(void *ctx, uint32_t page, const void *in)
{
    struct cpx_device *d = ctx;
    if (page >= STORE_IMAGE_PAGES)
        return 1;
    if (d->owner)
        CHECK(!d->owner->durable);
    ++d->operation;
    if (d->cut == d->operation)
        return 1;
    memcpy(d->working[page], in, STORE_PAGE_SIZE);
    return 0;
}

static int flush(void *ctx)
{
    struct cpx_device *d = ctx;
    if (d->owner)
        CHECK(!d->owner->durable);
    ++d->operation;
    if (d->cut == d->operation && !d->persist_before_error)
        return 1;
    memcpy(d->stable, d->working, sizeof d->stable);
    return d->cut == d->operation;
}

static store_io api(void)
{
    return (store_io){&disk, read_page, write_page, flush, STORE_IMAGE_PAGES};
}

static void queue(struct cp_queue *q)
{
    struct cp_message m = {0};
    m.sequence = 5;
    m.length = 4;
    m.payload[0] = 7;
    struct cp_parser parser;
    cp_parser_init(&parser);
    parser.ready = 1;
    parser.message = m;
    cp_queue_init(q);
    CHECK(cp_deliver(&parser, q) == CP_OK);
}

static unsigned reference(void)
{
    store s;
    CHECK(store_open(&s, api()) == STORE_OK);
    struct cp_queue q;
    queue(&q);
    struct cpx_sink sink = {0};
    sink.storage = &s;
    disk.owner = &sink;
    disk.operation = 0;
    int32_t output = 99;
    CHECK(cpx_sink_accept(&sink, &q) == SDR_OK && !q.count && sink.active &&
          !sink.durable);
    CHECK(cpx_sink_acknowledge(&sink, &output) == SDR_UNCERTAIN &&
          output == 99);
    CHECK(cpx_sink_commit(&sink) == SDR_OK && sink.durable);
    unsigned operations = disk.operation;
    CHECK(operations > 0);
    CHECK(cpx_sink_acknowledge(&sink, &output) == SDR_OK && output == 7 &&
          !sink.active);
    CHECK(cpx_sink_acknowledge(&sink, &output) == SDR_INVALID);
    disk.owner = NULL;
    return operations;
}

int main(void)
{
    CHECK(store_format(api()) == STORE_OK);
    store initial;
    CHECK(store_open(&initial, api()) == STORE_OK &&
          sdr_initialize(&initial) == SDR_OK);
    snapshot = disk;
    unsigned operations = reference(), cuts = 0;
    for (unsigned mode = 0; mode < 2; ++mode)
        for (unsigned cut = 1; cut <= operations; ++cut)
        {
            disk = snapshot;
            disk.cut = 0;
            disk.persist_before_error = mode;
            disk.operation = 0;
            store s;
            CHECK(store_open(&s, api()) == STORE_OK);
            disk.operation = 0;
            disk.cut = cut;
            struct cpx_sink sink = {0};
            sink.storage = &s;
            disk.owner = &sink;
            struct cp_queue q;
            queue(&q);
            CHECK(cpx_sink_accept(&sink, &q) == SDR_OK);
            int32_t output = 99;
            CHECK(cpx_sink_commit(&sink) != SDR_OK);
            CHECK(sink.active && !sink.durable && sink.held.sequence == 5);
            CHECK(cpx_sink_acknowledge(&sink, &output) == SDR_UNCERTAIN &&
                  output == 99);
            /* Simulated reset discards working state; only successful flush or declared
               persist-before-error bytes survive. This is not a physical power cut. */
            disk.owner = NULL;
            memcpy(disk.working, disk.stable, sizeof disk.working);
            disk.cut = 0;
            disk.operation = 0;
            store recovered;
            CHECK(store_open(&recovered, api()) == STORE_OK);
            sink.storage = &recovered;
            disk.owner = &sink;
            CHECK(cpx_sink_commit(&sink) == SDR_OK &&
                  cpx_sink_acknowledge(&sink, &output) == SDR_OK &&
                  output == 7);
            queue(&q);
            CHECK(cpx_sink_accept(&sink, &q) == SDR_OK &&
                  cpx_sink_commit(&sink) == SDR_OK &&
                  cpx_sink_acknowledge(&sink, &output) == SDR_OK &&
                  output == 7);
            int32_t effect;
            CHECK(sdr_value(&recovered, &effect) == SDR_OK && effect == 7);
            disk.owner = NULL;
            ++cuts;
        }
    printf(
        "CPX_PASS queued/held/written/durable/acknowledged phases; %u flush/write cuts; %u operations\n",
        cuts, operations);
    return 0;
}
