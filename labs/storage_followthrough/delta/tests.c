#include "delta.h"
#include "medium.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "SD_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)
static struct d_medium base, work, committed;
static struct sd_store subject, committed_subject;

static void maximum_generation(struct d_medium *m)
{
    unsigned selected = m->stable[0][8] >= m->stable[1][8] ? 0 : 1;
    uint8_t record[4096];
    memcpy(record, m->stable[selected], 4096);
    memset(record + 8, 255, 8);
    uint32_t c = UINT32_MAX;
    for (unsigned i = 0; i < 4092; i++)
    {
        c ^= record[i];
        for (unsigned j = 0; j < 8; j++)
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
    }
    c = ~c;
    for (unsigned i = 0; i < 4; i++)
        record[4092 + i] = (uint8_t)(c >> (i * 8));
    memcpy(m->stable[0], record, 4096);
    memcpy(m->stable[1], record, 4096);
}

static void reset(struct d_medium *m)
{
    dm_reboot(m);
    m->reads = m->writes = m->flushes = 0;
}

static int image(struct sd_store *s, unsigned state)
{
    uint8_t bytes[4096], expected[4096];
    for (unsigned p = 0; p < 32; p++)
    {
        memset(expected, 0, sizeof expected);
        if (p == 0)
            expected[0] = (uint8_t)(state ? 11 : 10);
        if (p == 1)
            expected[0] = (uint8_t)(state ? 0 : 20);
        if (p == 31)
            expected[0] = (uint8_t)(state ? 77 : 30);
        CHECK(sd_read(s, p, bytes) == SD_OK && !memcmp(bytes, expected, 4096));
    }
    return 0;
}

static int change(struct sd_store *s, unsigned newer)
{
    unsigned pages[] = {0, 1, 31};
    uint8_t old[] = {10, 20, 30}, next[] = {11, 0, 77};
    CHECK(sd_begin(s) == SD_OK);
    for (unsigned i = 0; i < 3; i++)
        CHECK(sd_write(s, pages[i], 0, newer ? next + i : old + i, 1) == SD_OK);
    return 0;
}

static int setup(unsigned full)
{
    dm_init(&base);
    CHECK(sd_format(dm_io(&base)) == SD_OK);
    reset(&base);
    CHECK(sd_open(&subject, dm_io(&base), full) == SD_OK);
    CHECK(change(&subject, 0) == 0);
    CHECK(sd_commit(&subject) == SD_OK);
    CHECK(sd_checkpoint(&subject) == SD_OK);
    reset(&base);
    CHECK(image(&subject, 0) == 0);
    reset(&base);
    return 0;
}

static int staged(unsigned full)
{
    work = base;
    CHECK(sd_open(&subject, dm_io(&work), full) == SD_OK);
    CHECK(change(&subject, 1) == 0);
    CHECK(dm_arm(&work, 0, DM_NONE, 0, 0, 0) == 0);
    work.reads = work.writes = work.flushes = 0;
    return 0;
}

static int boundary(unsigned full)
{
    CHECK(setup(full) == 0);
    work = base;
    CHECK(sd_open(&subject, dm_io(&work), full) == SD_OK);
    uint8_t v = 77, out[4096];
    CHECK(sd_write(&subject, 0, 0, &v, 1) == SD_STATE);
    CHECK(sd_begin(&subject) == SD_OK);
    struct sd_store saved = subject;
    CHECK(sd_write(&subject, 32, 0, &v, 1) == SD_INVALID &&
          !memcmp(&saved, &subject, sizeof subject));
    CHECK(sd_write(&subject, 0, 4096, &v, 1) == SD_INVALID &&
          !memcmp(&saved, &subject, sizeof subject));
    CHECK(sd_write(&subject, 0, SIZE_MAX, &v, 1) == SD_INVALID &&
          !memcmp(&saved, &subject, sizeof subject));
    CHECK(sd_write(&subject, 0, 4096, NULL, 0) == SD_OK);
    uint64_t w = work.writes, f = work.flushes;
    CHECK(sd_commit(&subject) == SD_OK && work.writes == w &&
          work.flushes == f);
    CHECK(sd_begin(&subject) == SD_OK);
    CHECK(sd_write(&subject, 31, 4095, &v, 1) == SD_OK);
    sd_abort(&subject);
    CHECK(image(&subject, 0) == 0);
    CHECK(sd_begin(&subject) == SD_OK);
    CHECK(sd_write(&subject, 31, 0, &v, 1) == SD_OK);
    CHECK(sd_commit(&subject) == SD_OK);
    reset(&work);
    CHECK(sd_open(&subject, dm_io(&work), full) == SD_OK);
    CHECK(sd_read(&subject, 31, out) == SD_OK && out[0] == 77 &&
          out[4095] == 0);
    reset(&work);
    memset(out, 0xa5, sizeof out);
    work.read_cut = 1;
    work.short_read = 1;
    CHECK(sd_read(&subject, 0, out) == SD_IO);
    for (unsigned i = 0; i < 4096; i++)
        CHECK(out[i] == 0xa5);
    reset(&work);
    maximum_generation(&work);
    CHECK(sd_open(&subject, dm_io(&work), full) == SD_OK &&
          subject.generation == UINT64_MAX);
    CHECK(sd_begin(&subject) == SD_OK);
    CHECK(sd_write(&subject, 1, 0, &v, 1) == SD_OK);
    w = work.writes;
    f = work.flushes;
    CHECK(sd_commit(&subject) == SD_FULL && !subject.active &&
          work.writes == w && work.flushes == f);
    CHECK(sd_begin(&subject) == SD_OK);
    CHECK(sd_commit(&subject) == SD_OK);
    printf("SD boundary validated policy=%u\n", full);
    return 0;
}

static int cuts(unsigned full, int checkpoint, int emit)
{
    CHECK(setup(full) == 0);
    CHECK(staged(full) == 0);
    unsigned events = full ? 37 : 8;
    if (checkpoint)
    {
        CHECK(sd_commit(&subject) == SD_OK);
        committed = work;
        committed_subject = subject;
    }
    for (unsigned mode = 0; mode < 7; mode++)
        for (unsigned cut = 1; cut <= events; cut++)
        {
            if (checkpoint)
            {
                work = committed;
                subject = committed_subject;
                subject.io = dm_io(&work);
                subject.poisoned = 0;
                subject.active = 0;
                subject.wal_mask = full ? UINT32_MAX : UINT32_C(0x80000003);
            }
            else
                CHECK(staged(full) == 0);
            CHECK(dm_arm(&work, cut, mode, 0, 0, 0) == 0);
            int r = checkpoint ? sd_checkpoint(&subject) : sd_commit(&subject);
            CHECK(r == (checkpoint          ? SD_IO
                        : cut >= events - 1 ? SD_UNCERTAIN
                                            : SD_IO));
            CHECK(subject.poisoned);
            CHECK(sd_begin(&subject) == SD_STATE);
            reset(&work);
            CHECK(sd_open(&subject, dm_io(&work), full) == SD_OK);
            unsigned newest =
                checkpoint ||
                (cut >= events - 1 && (mode == DM_EVEN || mode == DM_ALL));
            CHECK(image(&subject, newest) == 0);
            if (emit)
                printf("%u,%u,%u,%d,%u\n", full, mode, cut, r, newest);
            uint8_t homes[32][4096];
            memcpy(homes, work.stable + 2, sizeof homes);
            reset(&work);
            CHECK(sd_open(&subject, dm_io(&work), full) == SD_OK &&
                  !memcmp(homes, work.stable + 2, sizeof homes));
            CHECK(work.writes == 0);
        }
    if (!emit)
        printf("SD %s validated policy=%u cuts=%u full-image-checked\n",
               checkpoint ? "checkpoint" : "commit", full, events * 7);
    return 0;
}

static int repeated(unsigned full)
{
    CHECK(setup(full) == 0);
    CHECK(staged(full) == 0);
    CHECK(sd_commit(&subject) == SD_OK);
    committed = work;
    committed_subject = subject;
    unsigned events = full ? 37 : 8, histories = 0;
    for (unsigned first = 1; first <= events; first++)
        for (unsigned second = 1; second <= events; second++)
        {
            work = committed;
            subject = committed_subject;
            subject.io = dm_io(&work);
            subject.active = 0;
            subject.poisoned = 0;
            subject.wal_mask = full ? UINT32_MAX : UINT32_C(0x80000003);
            CHECK(dm_arm(&work, first, DM_NONE, 0, 0, 0) == 0);
            CHECK(sd_checkpoint(&subject) == SD_IO);
            reset(&work);
            CHECK(dm_arm(&work, second, DM_ALL, 0, 0, 0) == 0);
            int r = sd_open(&subject, dm_io(&work), full);
            CHECK(r == SD_IO || r == SD_OK);
            reset(&work);
            CHECK(sd_open(&subject, dm_io(&work), full) == SD_OK);
            CHECK(image(&subject, 1) == 0);
            histories++;
        }
    printf("SD repeated validated policy=%u histories=%u\n", full, histories);
    return 0;
}

static int corrupt(unsigned full)
{
    CHECK(setup(full) == 0);
    CHECK(staged(full) == 0);
    CHECK(sd_commit(&subject) == SD_OK);
    committed = work;
    committed_subject = subject;
    for (unsigned which = 0; which < 2; which++)
    {
        work = committed;
        subject = committed_subject;
        reset(&work);
        unsigned page = which && !full ? 19 : 65;
        work.stable[page][231] ^= 1;
        uint8_t homes[32][4096];
        memcpy(homes, work.stable + 2, sizeof homes);
        CHECK(sd_open(&subject, dm_io(&work), full) == SD_CORRUPT &&
              work.writes == 0 &&
              !memcmp(homes, work.stable + 2, sizeof homes));
    }
    for (unsigned fail = 1; fail <= 35; fail++)
    {
        work = committed;
        subject = committed_subject;
        reset(&work);
        work.read_cut = fail;
        work.short_read = 1;
        CHECK(sd_open(&subject, dm_io(&work), full) == SD_IO &&
              work.writes == 0);
    }
    printf("SD corrupt validated policy=%u preflight-before-home-publication\n",
           full);
    return 0;
}

static int cost(unsigned full, int emit)
{
    CHECK(setup(full) == 0);
    CHECK(staged(full) == 0);
    CHECK(sd_commit(&subject) == SD_OK);
    uint64_t cr = work.reads, cw = work.writes, cf = work.flushes;
    CHECK(cr == 0 && cw == (full ? 34 : 5) && cf == 3);
    CHECK(dm_arm(&work, 0, DM_NONE, 0, 0, 0) == 0);
    work.reads = work.writes = work.flushes = 0;
    CHECK(sd_checkpoint(&subject) == SD_OK);
    CHECK(work.reads == (full ? 65 : 36) && work.writes == (full ? 34 : 5) &&
          work.flushes == 3);
    CHECK(image(&subject, 1) == 0);
    if (emit)
        printf(
            "{\"full\":%u,\"commit\":[%llu,%llu,%llu],\"checkpoint\":[%u,%u,3],"
            "\"useful_bytes\":3}",
            full, (unsigned long long)cr, (unsigned long long)cw,
            (unsigned long long)cf, full ? 65 : 36, full ? 34 : 5);
    else
        printf("SD cost validated policy=%u same-transaction-contract\n", full);
    return 0;
}

static int page(unsigned full)
{
    CHECK(setup(full) == 0);
    CHECK(staged(full) == 0);
    CHECK(sd_commit(&subject) == SD_OK);
    for (unsigned i = 0; i < 4096; i++)
        printf("%02x", work.stable[66][i]);
    puts("");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "oracle"))
    {
        for (unsigned f = 0; f < 2; f++)
            CHECK(cuts(f, 0, 1) == 0);
        return 0;
    }
    if (!strcmp(argv[1], "pages"))
    {
        for (unsigned f = 0; f < 2; f++)
            CHECK(page(f) == 0);
        return 0;
    }
    if (!strcmp(argv[1], "metrics"))
    {
        putchar('[');
        CHECK(cost(0, 1) == 0);
        putchar(',');
        CHECK(cost(1, 1) == 0);
        puts("]");
        return 0;
    }
    for (unsigned f = 0; f < 2; f++)
    {
        int r = !strcmp(argv[1], "boundary")     ? boundary(f)
                : !strcmp(argv[1], "commit")     ? cuts(f, 0, 0)
                : !strcmp(argv[1], "checkpoint") ? cuts(f, 1, 0)
                : !strcmp(argv[1], "repeated")   ? repeated(f)
                : !strcmp(argv[1], "corrupt")    ? corrupt(f)
                : !strcmp(argv[1], "cost")       ? cost(f, 0)
                                                 : 2;
        if (r)
            return r;
    }
    return 0;
}
