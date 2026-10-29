#include "logical.h"
#include "medium.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "SL_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)
static struct d_medium initial, work, committed;
static struct sl_counter counter, saved;

static void reset(struct d_medium *m)
{
    dm_reboot(m);
    m->reads = m->writes = m->flushes = 0;
}

static void word(uint8_t *p, uint64_t v)
{
    for (unsigned i = 0; i < 8; i++)
    {
        p[i] = (uint8_t)(v >> (8 * i));
    }
}

static void seal(uint8_t *p)
{
    uint32_t c = UINT32_MAX;
    for (unsigned i = 0; i < 4092; i++)
    {
        c ^= p[i];
        for (unsigned j = 0; j < 8; j++)
        {
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
        }
    }
    c = ~c;
    for (unsigned i = 0; i < 4; i++)
    {
        p[4092 + i] = (uint8_t)(c >> (8 * i));
    }
}

static int setup(void)
{
    dm_init(&initial);
    CHECK(sl_format(dm_io(&initial)) == SL_OK);
    reset(&initial);
    CHECK(sl_open(&counter, dm_io(&initial)) == SL_OK);
    struct sl_receipt out;
    CHECK(sl_apply(&counter, 1, 10, &out) == SL_OK && out.id == 1 &&
          out.total == 10);
    CHECK(sl_checkpoint(&counter) == SL_OK);
    reset(&initial);
    return 0;
}

static int begin(void)
{
    work = initial;
    CHECK(sl_open(&counter, dm_io(&work)) == SL_OK);
    CHECK(dm_arm(&work, 0, DM_NONE, 0, 0, 0) == 0);
    return 0;
}

static int value(uint64_t id, int64_t total)
{
    struct sl_receipt out;
    CHECK(sl_get(&counter, &out) == SL_OK && out.id == id &&
          out.total == total);
    return 0;
}

static int cuts(int checkpoint, int emit)
{
    CHECK(setup() == 0);
    CHECK(begin() == 0);
    struct sl_receipt out = {999, 999};
    if (checkpoint)
    {
        CHECK(sl_apply(&counter, 2, 7, &out) == SL_OK);
        committed = work;
        saved = counter;
    }
    for (unsigned mode = 0; mode < 7; mode++)
    {
        for (unsigned cut = 1; cut <= 4; cut++)
        {
            if (checkpoint)
            {
                work = committed;
                counter = saved;
                counter.io = dm_io(&work);
            }
            else
            {
                CHECK(begin() == 0);
            }
            out = (struct sl_receipt){999, 999};
            CHECK(dm_arm(&work, cut, mode, 0, 0, 0) == 0);
            int r = checkpoint ? sl_checkpoint(&counter)
                               : sl_apply(&counter, 2, 7, &out);
            CHECK(r == (checkpoint ? SL_IO : cut >= 3 ? SL_UNCERTAIN : SL_IO));
            CHECK(counter.poisoned && out.id == 999 && out.total == 999);
            CHECK(sl_get(&counter, &out) == SL_STATE);
            reset(&work);
            CHECK(sl_open(&counter, dm_io(&work)) == SL_OK);
            unsigned newest =
                checkpoint || (cut >= 3 && (mode == DM_EVEN || mode == DM_ALL));
            CHECK(value(newest ? 2 : 1, newest ? 17 : 10) == 0);
            uint64_t writes = work.writes, flushes = work.flushes;
            CHECK(sl_apply(&counter, 2, 7, &out) ==
                  (newest ? SL_DUPLICATE : SL_OK));
            CHECK(out.id == 2 && out.total == 17);
            if (newest)
            {
                CHECK(work.writes == writes && work.flushes == flushes);
            }
            if (emit)
            {
                printf("%u,%u,%d,%u\n", mode, cut, r, newest);
            }
        }
    }
    if (!emit)
    {
        printf("SL %s validated cuts=28 exact-once-counter-under-ID-policy\n",
               checkpoint ? "checkpoint" : "commit");
    }
    return 0;
}

static int repeated(void)
{
    CHECK(setup() == 0);
    CHECK(begin() == 0);
    struct sl_receipt out;
    CHECK(sl_apply(&counter, 2, 7, &out) == SL_OK);
    committed = work;
    saved = counter;
    unsigned histories = 0;
    for (unsigned mode = 0; mode < 7; mode++)
    {
        for (unsigned first = 1; first <= 4; first++)
        {
            for (unsigned second = 1; second <= 4; second++)
            {
                work = committed;
                counter = saved;
                counter.io = dm_io(&work);
                CHECK(dm_arm(&work, first, mode, 0, 0, 0) == 0);
                CHECK(sl_checkpoint(&counter) == SL_IO);
                reset(&work);
                CHECK(dm_arm(&work, second, DM_ALL, 0, 0, 0) == 0);
                int r = sl_open(&counter, dm_io(&work));
                CHECK(r == SL_OK || r == SL_IO);
                reset(&work);
                CHECK(sl_open(&counter, dm_io(&work)) == SL_OK);
                CHECK(value(2, 17) == 0);
                CHECK(sl_apply(&counter, 2, 7, &out) == SL_DUPLICATE &&
                      out.total == 17);
                histories++;
            }
        }
    }
    printf(
        "SL repeated validated histories=%u sequence-skips-applied-command\n",
        histories);
    return 0;
}

static int identity(void)
{
    CHECK(setup() == 0);
    CHECK(begin() == 0);
    struct sl_receipt out = {888, -888};
    uint64_t writes = work.writes, flushes = work.flushes;
    CHECK(sl_apply(&counter, 0, 1, &out) == SL_INVALID);
    CHECK(sl_apply(&counter, 3, 1, &out) == SL_GAP);
    CHECK(sl_apply(&counter, 1, 11, &out) == SL_CONFLICT);
    CHECK(out.id == 888 && out.total == -888 && work.writes == writes &&
          work.flushes == flushes);
    CHECK(sl_apply(&counter, 1, 10, &out) == SL_DUPLICATE && out.total == 10);
    CHECK(work.writes == writes && work.flushes == flushes);
    CHECK(sl_apply(&counter, 2, 7, &out) == SL_OK);
    reset(&work);
    CHECK(sl_open(&counter, dm_io(&work)) == SL_OK);
    CHECK(sl_apply(&counter, 2, 7, &out) == SL_DUPLICATE && out.total == 17);
    CHECK(sl_apply(&counter, 1, 10, &out) == SL_STALE);
    CHECK(sl_apply(&counter, 3, 7, &out) == SL_OK &&
          out.total ==
              24); /* Blind new-ID retry really duplicates the effect. */
    printf("SL identity validated lost-reply-same-ID17 blind-new-ID24 "
           "stale-conflict-gap\n");
    return 0;
}

static int limits(void)
{
    CHECK(setup() == 0);
    CHECK(begin() == 0);
    struct sl_receipt out = {999, 999};
    CHECK(sl_apply(&counter, 2, INT64_MAX - 10, &out) == SL_OK &&
          out.total == INT64_MAX);
    out = (struct sl_receipt){999, 999};
    uint64_t w = work.writes, f = work.flushes;
    CHECK(sl_apply(&counter, 3, 1, &out) == SL_FULL && out.id == 999 &&
          out.total == 999 && work.writes == w && work.flushes == f);
    CHECK(sl_apply(&counter, 3, INT64_MIN, &out) == SL_OK && out.total == -1);
    CHECK(sl_apply(&counter, 4, INT64_MIN, &out) == SL_FULL);
    CHECK(sl_apply(&counter, 4, INT64_MIN + 1, &out) == SL_OK &&
          out.total == INT64_MIN);
    CHECK(sl_checkpoint(&counter) == SL_OK);
    reset(&work);
    uint8_t bytes[4096];
    memcpy(bytes, work.stable[0], 4096);
    word(bytes + 8, UINT64_MAX);
    seal(bytes);
    memcpy(work.stable[0], bytes, 4096);
    memcpy(work.stable[1], bytes, 4096);
    CHECK(sl_open(&counter, dm_io(&work)) == SL_OK && counter.id == UINT64_MAX);
    CHECK(sl_apply(&counter, UINT64_MAX, INT64_MIN + 1, &out) == SL_DUPLICATE);
    CHECK(sl_apply(&counter, 0, 1, &out) == SL_INVALID);
    CHECK(sl_apply(&counter, 1, 1, &out) == SL_STALE);
    CHECK(sl_checkpoint(&counter) == SL_OK);
    printf("SL limits validated checked-int64-and-no-ID-wrap\n");
    return 0;
}

static int corruption(void)
{
    CHECK(setup() == 0);
    CHECK(begin() == 0);
    struct sl_receipt out;
    CHECK(sl_apply(&counter, 2, 7, &out) == SL_OK);
    committed = work;
    for (unsigned which = 0; which < 3; which++)
    {
        work = committed;
        reset(&work);
        if (which == 0)
        {
            word(work.stable[2] + 8, 3);
            word(work.stable[2] + 16, 2);
            seal(work.stable[2]);
        }
        else if (which == 1)
        {
            word(work.stable[2] + 24, INT64_MAX);
            seal(work.stable[2]);
        }
        else
        {
            memcpy(work.stable[0], work.stable[1], 4096);
            work.stable[0][16] ^= 1;
            seal(work.stable[0]);
        }
        uint8_t homes[2][4096];
        memcpy(homes, work.stable, sizeof homes);
        CHECK(sl_open(&counter, dm_io(&work)) == SL_CORRUPT &&
              work.writes == 0 && !memcmp(homes, work.stable, sizeof homes));
    }
    for (unsigned cut = 1; cut <= 3; cut++)
    {
        work = committed;
        reset(&work);
        work.read_cut = cut;
        work.short_read = 1;
        CHECK(sl_open(&counter, dm_io(&work)) == SL_IO && work.writes == 0);
    }
    printf(
        "SL corruption validated resealed-gap-overflow-metadata-disagreement\n");
    return 0;
}

static int histories(int emit)
{
    CHECK(setup() == 0);
    unsigned count = 0, power = 1;
    for (unsigned n = 0; n <= 6; n++)
    {
        for (unsigned code = 0; code < power; code++)
        {
            CHECK(begin() == 0);
            unsigned rest = code;
            int64_t expected = 10;
            struct sl_receipt out;
            for (unsigned step = 0; step < n; step++)
            {
                int64_t delta = (int64_t)(rest % 3) - 1;
                rest /= 3;
                expected += delta;
                CHECK(sl_apply(&counter, step + 2, delta, &out) == SL_OK &&
                      out.total == expected);
                CHECK(sl_apply(&counter, step + 2, delta, &out) ==
                          SL_DUPLICATE &&
                      out.total == expected);
                reset(&work);
                CHECK(sl_open(&counter, dm_io(&work)) == SL_OK);
                CHECK(value(step + 2, expected) == 0);
            }
            if (emit)
            {
                printf("%u,%u,%lld\n", n, code, (long long)expected);
            }
            count++;
        }
        power *= 3;
    }
    if (!emit)
    {
        printf("SL histories validated histories=%u "
               "reopen-after-every-logical-operation\n",
               count);
    }
    return 0;
}

static int page(void)
{
    CHECK(setup() == 0);
    CHECK(begin() == 0);
    struct sl_receipt out;
    CHECK(sl_apply(&counter, 2, -7, &out) == SL_OK);
    for (unsigned i = 0; i < 4096; i++)
    {
        printf("%02x", work.stable[2][i]);
    }
    puts("");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    const char *a = argv[1];
    return !strcmp(a, "commit")       ? cuts(0, 0)
           : !strcmp(a, "checkpoint") ? cuts(1, 0)
           : !strcmp(a, "repeated")   ? repeated()
           : !strcmp(a, "identity")   ? identity()
           : !strcmp(a, "limits")     ? limits()
           : !strcmp(a, "corruption") ? corruption()
           : !strcmp(a, "histories")  ? histories(0)
           : !strcmp(a, "oracle")     ? histories(1)
           : !strcmp(a, "cuts")       ? cuts(0, 1)
           : !strcmp(a, "page")       ? page()
                                      : 2;
}
