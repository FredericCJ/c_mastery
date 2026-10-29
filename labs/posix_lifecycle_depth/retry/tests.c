#include "receipt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "PL_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

/* Original finite device model: reads see the staged image; successful flush
 * persists it all. A chosen write/flush fails and ends access. At a failed
 * flush, mode1 persists first, then reports uncertainty. Neither is hardware. */
struct pt_image
{
    unsigned char stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE],
        pending[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
    unsigned event, cut, mode;
    int dead;
};
static struct pt_image image, baseline;
static store server;

static int read_page(void *ctx, uint32_t page, void *out)
{
    struct pt_image *m = ctx;
    if (m->dead || page >= STORE_IMAGE_PAGES)
    {
        return -1;
    }
    memcpy(out, m->pending[page], STORE_PAGE_SIZE);
    return 0;
}

static int write_page(void *ctx, uint32_t page, const void *in)
{
    struct pt_image *m = ctx;
    if (m->dead || page >= STORE_IMAGE_PAGES)
    {
        return -1;
    }
    if (++m->event == m->cut)
    {
        m->dead = 1;
        return -1;
    }
    memcpy(m->pending[page], in, STORE_PAGE_SIZE);
    return 0;
}

static int flush(void *ctx)
{
    struct pt_image *m = ctx;
    if (m->dead)
    {
        return -1;
    }
    if (++m->event == m->cut)
    {
        if (m->mode)
        {
            memcpy(m->stable, m->pending, sizeof m->stable);
        }
        m->dead = 1;
        return -1;
    }
    memcpy(m->stable, m->pending, sizeof m->stable);
    return 0;
}

static store_io device(void)
{
    return (store_io){&image, read_page, write_page, flush, STORE_IMAGE_PAGES};
}

static void restart(void)
{
    memcpy(image.pending, image.stable, sizeof image.pending);
    image.dead = 0;
    image.event = image.cut = 0;
    CHECK(store_open(&server, device()) == STORE_OK);
}

int main(void)
{
    int32_t value, out;
    unsigned blind = 0;
    blind += 7; /* reply discarded */
    blind += 7;
    CHECK(blind == 14);
    puts("PR_BLIND_RETRY effects=14 expected_single=7");
    CHECK(store_format(device()) == STORE_OK &&
          store_open(&server, device()) == STORE_OK);
    CHECK(sdr_initialize(&server) == SDR_OK &&
          store_checkpoint(&server) == STORE_OK);
    baseline = image;
    image.event = 0;
    CHECK(sdr_apply(&server, 1, 1, 5, 7, &out) == SDR_OK && out == 7);
    unsigned events = image.event;
    CHECK(events > 0 && events < 128);
    /* The reply is deliberately forgotten; a new service object retries. */
    restart();
    CHECK(sdr_apply(&server, 1, 1, 5, 7, &out) == SDR_OK && out == 7);
    CHECK(sdr_value(&server, &value) == SDR_OK && value == 7);
    unsigned cuts = 0;
    for (unsigned mode = 0; mode < 2; ++mode)
    {
        for (unsigned cut = 1; cut <= events; ++cut)
        {
            image = baseline;
            CHECK(store_open(&server, device()) == STORE_OK);
            image.event = 0;
            image.cut = cut;
            image.mode = mode;
            out = 99;
            int result = sdr_apply(&server, 1, 1, 5, 7, &out);
            CHECK((result == SDR_STORAGE || result == SDR_UNCERTAIN) &&
                  out == 99);
            restart();
            CHECK(sdr_value(&server, &value) == SDR_OK &&
                  (value == 0 || value == 7));
            CHECK(sdr_apply(&server, 1, 1, 5, 7, &out) == SDR_OK && out == 7);
            CHECK(sdr_value(&server, &value) == SDR_OK && value == 7 &&
                  store_check(&server) == STORE_OK);
            ++cuts;
        }
    }
    out = 99;
    CHECK(sdr_apply(&server, 1, 1, 5, 8, &out) == SDR_CONFLICT && out == 99);
    CHECK(sdr_apply(&server, 2, 1, 9, 3, &out) == SDR_OK && out == 10);
    CHECK(sdr_apply(&server, 1, 1, 10, -2, &out) == SDR_OK && out == 8);
    CHECK(sdr_apply(&server, 1, 1, 20, 1, &out) == SDR_OK && out == 9);
    CHECK(sdr_apply(&server, 1, 1, 30, 1, &out) == SDR_OK && out == 10);
    out = 99;
    CHECK(sdr_apply(&server, 1, 1, 5, 7, &out) == SDR_STALE && out == 99);
    CHECK(sdr_apply(&server, 1, 2, 40, 1, &out) == SDR_STALE && out == 99);
    CHECK(sdr_apply(&server, 2, 1, 9, 3, &out) == SDR_OK && out == 10);
    printf(
        "PR_RETRY cuts=%u events=%u models=2 final=10 retained=3_per_client stale_evicted_id=5\n",
        cuts, events);
    puts("PR_PASS");
    return 0;
}
