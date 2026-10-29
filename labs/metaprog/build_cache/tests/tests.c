#include "build_cache.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "MPO_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static void nodes(struct mpb_node n[3])
{
    memset(n, 0, 3 * sizeof *n);
    for (unsigned i = 0; i < 3; ++i)
    {
        n[i].id = i;
        n[i].revision = (int32_t)i - 1;
        for (unsigned j = 0; j < 6; ++j)
        {
            n[i].inputs[j].length = 1;
            n[i].inputs[j].bytes[0] = (unsigned char)(10 * i + j);
        }
    }
    n[1].dependencies = 1;
}

static int inputs(void)
{
    struct mpb_node n[3], changed[3];
    struct mpb_plan p, q;
    nodes(n);
    CHECK(mpb_plan(n, 3, 0, &p) == MPB_OK && p.order[0] == 0 &&
          p.order[1] == 1 && p.order[2] == 2);
    for (unsigned j = 0; j < 6; ++j)
    {
        memcpy(changed, n, sizeof n);
        ++changed[0].inputs[j].bytes[0];
        CHECK(mpb_plan(changed, 3, 0, &q) == MPB_OK &&
              !mpb_match(&p.entries[0], &q.entries[0]) &&
              !mpb_match(&p.entries[1], &q.entries[1]) &&
              mpb_match(&p.entries[2], &q.entries[2]));
    }
    changed[0] = n[2];
    changed[1] = n[0];
    changed[2] = n[1];
    CHECK(mpb_plan(changed, 3, 0, &q) == MPB_OK);
    for (unsigned i = 0; i < 3; ++i)
    {
        CHECK(mpb_match(&p.entries[i], &q.entries[i]));
    }
    CHECK(mpb_plan(n, 3, 1, &p) == MPB_OK);
    ++n[0].revision;
    CHECK(mpb_plan(n, 3, 1, &q) == MPB_OK &&
          p.entries[0].key == q.entries[0].key &&
          !mpb_match(&p.entries[0], &q.entries[0]));
    struct mpb_plan old = q;
    n[0].dependencies = 2;
    CHECK(mpb_plan(n, 3, 0, &q) == MPB_CYCLE && !memcmp(&q, &old, sizeof q));
    n[0].dependencies = 0;
    n[0].id = 1;
    CHECK(mpb_plan(n, 3, 0, &q) == MPB_INVALID && !memcmp(&q, &old, sizeof q));
    puts(
        "MPB_INPUTS full-transitive-inputs stable-logical-ids forced-collision-refusal");
    return 0;
}

static int schedule(void)
{
    struct mpb_node n[3];
    struct mpb_plan p, out = {0};
    struct mpb_schedule s = {0};
    struct mpb_ticket a, b, c, old;
    nodes(n);
    CHECK(mpb_plan(n, 3, 0, &p) == MPB_OK && mpb_begin(&s, &p) == MPB_OK);
    CHECK(mpb_claim(&s, &a) == MPB_OK && a.id == 0);
    CHECK(mpb_claim(&s, &b) == MPB_OK && b.id == 2);
    CHECK(mpb_claim(&s, &c) == MPB_BUSY);
    CHECK(mpb_publish(&s, &out) == MPB_BUSY && out.count == 0);
    CHECK(mpb_finish(&s, a, &p.entries[0]) == MPB_OK &&
          mpb_finish(&s, a, &p.entries[0]) == MPB_STALE);
    CHECK(mpb_claim(&s, &c) == MPB_OK && c.id == 1);
    CHECK(mpb_finish(&s, c, &p.entries[1]) == MPB_OK);
    CHECK(mpb_finish(&s, b, &p.entries[2]) == MPB_OK &&
          mpb_publish(&s, &out) == MPB_OK && out.count == 3);
    old = a;
    CHECK(mpb_begin(&s, &p) == MPB_OK &&
          mpb_finish(&s, old, &p.entries[0]) == MPB_STALE);
    CHECK(mpb_claim(&s, &a) == MPB_OK && mpb_claim(&s, &b) == MPB_OK);
    CHECK(mpb_finish(&s, a, NULL) == MPB_FAILED &&
          mpb_claim(&s, &c) == MPB_FAILED && mpb_publish(&s, &out) == MPB_BUSY);
    CHECK(mpb_finish(&s, b, &p.entries[2]) == MPB_OK &&
          mpb_publish(&s, &out) == MPB_FAILED);
    s.epoch = UINT64_MAX;
    CHECK(mpb_begin(&s, &p) == MPB_EXHAUSTED);
    puts(
        "MPB_SCHEDULE dependency-admission exclusive-tickets drain-before-publication");
    return 0;
}

static int publication(void)
{
    struct mpb_node n[3];
    struct mpb_plan p;
    struct mpb_entry bad;
    unsigned char bytes[800], old[800];
    size_t used = 99;
    nodes(n);
    CHECK(mpb_plan(n, 3, 0, &p) == MPB_OK);
    bad = p.entries[0];
    bad.artifact[10] ^= 1;
    CHECK(!mpb_match(&p.entries[0], &bad));
    memset(bytes, 0xa5, sizeof bytes);
    memcpy(old, bytes, sizeof bytes);
    CHECK(mpb_encode(&p.entries[0], 0, bytes, 1, &used) == MPB_FULL &&
          used == 99 && !memcmp(bytes, old, sizeof bytes));
    CHECK(mpb_encode(&p.entries[0], 0, bytes, sizeof bytes, &used) == MPB_OK &&
          used ==
              17 + p.entries[0].input_length + p.entries[0].artifact_length &&
          bytes[0] == 'M' && bytes[3] == 1 && bytes[4] == 0 &&
          bytes[used] == 0xa5);
    puts(
        "MPB_PUBLICATION canonical-entry exact-artifact-check bounded-output-transaction");
    return 0;
}

static int base(void)
{
    unsigned char x[] = {1}, y[] = {2};
    struct mp_build_node n[] = {{0, x, 1, 3, 4}, {1, y, 1, 3, 4}};
    unsigned order[2];
    uint64_t keys[2], changed[2];
    char text[256];
    size_t used = 0;
    CHECK(mp_build(n, 2, order, keys));
    ++n[0].configuration;
    CHECK(mp_build(n, 2, order, changed) && keys[0] != changed[0] &&
          keys[1] != changed[1]);
    uint64_t full = UINT64_C(0xfedcba9876543210);
    CHECK(mp_emit_table(&full, 1, text, sizeof text, &used) &&
          strstr(text, "fedcba9876543210"));
    puts(
        "MPB_BASE actual-topology configuration-and-dependency-keys complete-table-width");
    return 0;
}

static void printhex(const unsigned char *b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        printf("%02x", b[i]);
    }
    if (!n)
    {
        putchar('-');
    }
}

static int oracle(void)
{
    size_t count;
    unsigned force;
    while (scanf("%zu %u", &count, &force) == 2)
    {
        struct mpb_node n[8] = {0};
        struct mpb_plan p = {0};
        p.count = 777;
        if (count > 8)
        {
            return 2;
        }
        for (size_t i = 0; i < count; ++i)
        {
            if (scanf("%u %u %" SCNd32, &n[i].id, &n[i].dependencies,
                      &n[i].revision) != 3)
            {
                return 2;
            }
            for (unsigned j = 0; j < 6; ++j)
            {
                char h[19];
                if (scanf("%18s", h) != 1)
                {
                    return 2;
                }
                n[i].inputs[j].length = !strcmp(h, "-") ? 0 : strlen(h) / 2;
                for (size_t k = 0; k < n[i].inputs[j].length && k < 8; ++k)
                {
                    unsigned b;
                    if (sscanf(h + 2 * k, "%2x", &b) != 1)
                    {
                        return 2;
                    }
                    n[i].inputs[j].bytes[k] = (unsigned char)b;
                }
            }
        }
        int status = mpb_plan(n, count, force, &p);
        printf("%d %zu", status, p.count);
        if (status == MPB_OK)
        {
            for (size_t i = 0; i < count; ++i)
            {
                printf(" %u", p.order[i]);
            }
            for (size_t i = 0; i < count; ++i)
            {
                printf(" %" PRIu64 " ", p.entries[i].key);
                printhex(p.entries[i].input, p.entries[i].input_length);
                putchar(' ');
                printhex((const unsigned char *)p.entries[i].artifact,
                         p.entries[i].artifact_length);
            }
        }
        putchar('\n');
    }
    return 0;
}

static int metrics(void)
{
    struct mpb_node n[3];
    nodes(n);
    struct mpb_plan p;
    double sec[7];
    uint64_t sum = 0;
    for (unsigned s = 0; s < 7; ++s)
    {
        clock_t start = clock();
        for (unsigned i = 0; i < 10000; ++i)
        {
            CHECK(mpb_plan(n, 3, 0, &p) == MPB_OK);
            sum += p.count;
        }
        sec[s] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf("{\"iterations\":10000,\"checksum\":%" PRIu64
           ",\"plan_size\":%zu,\"entry_size\":%zu,\"seconds\":[",
           sum, sizeof p, sizeof(struct mpb_entry));
    for (unsigned i = 0; i < 7; ++i)
    {
        printf("%s%.9f", i ? "," : "", sec[i]);
    }
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "cache_inputs"))
    {
        return inputs();
    }
    if (!strcmp(argv[1], "cache_schedule"))
    {
        return schedule();
    }
    if (!strcmp(argv[1], "cache_publication"))
    {
        return publication();
    }
    if (!strcmp(argv[1], "cache_base"))
    {
        return base();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        return metrics();
    }
    return 2;
}
