/* Reuse the retained explicit five-mode device fault model, not its oracles. */
#define main retained_store_test_main
#include "../../../labs/storage/tests/test_store.c"
#undef main
#include "receipt.h"
/* A second model has a different persistence granularity: each of eight512-byte
 * sectors may retain old bytes or take new bytes. At a failed flush, dirty pages
 * are visited in reverse order; a prefix may persist and the next page may tear.
 * Enumeration is all masks for the addressed write, plus all page-prefix lengths
 * at flush with all masks for its boundary page. It is NOT all joint masks across
 * all dirty pages, and never supplies a proof against adversarial checksum forgery. */
static unsigned sector_mask, flush_prefix;
static unsigned recording, event_prefixes[128];

static void tear(medium *m, unsigned page)
{
    for (unsigned sector = 0; sector < 8; sector++)
        if (sector_mask & (1u << sector))
            memcpy(m->stable[page] + 512 * sector,
                   m->pending[page] + 512 * sector, 512);
}

static int sector_boundary(medium *m, unsigned page, int writing)
{
    ++m->event;
    if (recording && m->event < 128)
    {
        unsigned dirty = 0;
        for (unsigned i = 0; i < STORE_IMAGE_PAGES; i++)
            dirty += m->dirty[i] != 0;
        event_prefixes[m->event] = writing ? 0 : dirty;
    }
    if (m->event != m->cut || !m->cut)
        return 0;
    if (writing)
        tear(m, page);
    else
    {
        unsigned visited = 0;
        for (unsigned i = STORE_IMAGE_PAGES; i; i--)
            if (m->dirty[i - 1])
            {
                if (visited < flush_prefix)
                    memcpy(m->stable[i - 1], m->pending[i - 1],
                           STORE_PAGE_SIZE);
                else if (visited == flush_prefix)
                    tear(m, i - 1);
                visited++;
            }
    }
    m->dead = 1;
    return -1;
}

static int sector_write(void *ctx, uint32_t page, const void *bytes)
{
    medium *m = ctx;
    if (m->dead || page >= STORE_IMAGE_PAGES)
        return -1;
    memcpy(m->pending[page], bytes, STORE_PAGE_SIZE);
    m->dirty[page] = 1;
    return sector_boundary(m, page, 1);
}

static int sector_flush(void *ctx)
{
    medium *m = ctx;
    if (m->dead)
        return -1;
    if (sector_boundary(m, 0, 0))
        return -1;
    persist(m);
    return 0;
}

static store_io sector_io(medium *m)
{
    store_io io = {m, mread, sector_write, sector_flush, STORE_IMAGE_PAGES};
    return io;
}

static int sector_cases(void)
{
    static store s;
    int32_t out, value;
    memset(&disk, 0, sizeof disk);
    CHECK(!store_format(sector_io(&disk)));
    CHECK(!store_open(&s, sector_io(&disk)));
    CHECK(!sdr_initialize(&s));
    CHECK(!store_checkpoint(&s));
    base = disk;
    disk.event = 0;
    recording = 1;
    CHECK(!sdr_apply(&s, 1, 1, 5, 7, &out));
    recording = 0;
    unsigned events = disk.event, cases = 0;
    CHECK(events < 128);
    for (unsigned cut = 1; cut <= events; cut++)
        for (sector_mask = 0; sector_mask < 256; sector_mask++)
            for (flush_prefix = 0; flush_prefix <= event_prefixes[cut];
                 flush_prefix++)
            {
                disk = base;
                CHECK(!store_open(&s, sector_io(&disk)));
                disk.event = 0;
                disk.cut = cut;
                out = 99;
                int result = sdr_apply(&s, 1, 1, 5, 7, &out);
                CHECK(result == SDR_STORAGE || result == SDR_UNCERTAIN);
                CHECK(out == 99);
                crash(&disk);
                CHECK(!store_open(&s, sector_io(&disk)));
                CHECK(!sdr_value(&s, &value) && (value == 0 || value == 7));
                CHECK(!sdr_apply(&s, 1, 1, 5, 7, &out) && out == 7);
                CHECK(!sdr_value(&s, &value) && value == 7);
                CHECK(!store_check(&s));
                cases++;
            }
    printf(
        "PASS receipt-sector-model cases=%u events=%u masks=256 prefixes=all-dirty-page-boundaries\n",
        cases, events);
    return 0;
}

static int receipts(void)
{
    static store s;
    int32_t out = 99, value;
    memset(&disk, 0, sizeof disk);
    CHECK(store_format(mio(&disk)) == STORE_OK);
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(!sdr_initialize(&s));
    CHECK(store_checkpoint(&s) == STORE_OK);
    base = disk;
    disk.event = 0;
    CHECK(!sdr_apply(&s, 1, 1, 5, 7, &out) && out == 7);
    unsigned events = disk.event;
    CHECK(events >= 37);
    unsigned cases = 0;
    for (unsigned mode = 0; mode < 5; mode++)
        for (unsigned cut = 1; cut <= events; cut++)
        {
            disk = base;
            CHECK(store_open(&s, mio(&disk)) == STORE_OK);
            disk.event = 0;
            disk.cut = cut;
            disk.mode = mode;
            out = 99;
            int result = sdr_apply(&s, 1, 1, 5, 7, &out);
            CHECK(result == SDR_STORAGE || result == SDR_UNCERTAIN);
            CHECK(out == 99);
            crash(&disk);
            CHECK(store_open(&s, mio(&disk)) == STORE_OK);
            CHECK(!sdr_value(&s, &value) && (value == 0 || value == 7));
            CHECK(!sdr_apply(&s, 1, 1, 5, 7, &out) && out == 7);
            CHECK(!sdr_value(&s, &value) && value == 7);
            CHECK(store_check(&s) == STORE_OK);
            cases++;
        }
    out = 99;
    CHECK(sdr_apply(&s, 1, 1, 5, 8, &out) == SDR_CONFLICT && out == 99);
    CHECK(!sdr_apply(&s, 2, 1, 9, 3, &out) && out == 10);
    CHECK(!sdr_apply(&s, 1, 1, 10, -2, &out) && out == 8);
    CHECK(!sdr_apply(&s, 1, 1, 20, 1, &out) && out == 9);
    CHECK(!sdr_apply(&s, 1, 1, 30, 1, &out) && out == 10);
    CHECK(sdr_apply(&s, 1, 1, 5, 7, &out) == SDR_STALE);
    CHECK(sdr_apply(&s, 1, 2, 40, 1, &out) == SDR_STALE);
    printf("PASS receipt-fault-model cases=%u events=%u\n", cases, events);
    return 0;
}

int main(int argc, char **argv)
{
    return argc == 2 && !strcmp(argv[1], "sectors") ? sector_cases()
                                                    : receipts();
}
