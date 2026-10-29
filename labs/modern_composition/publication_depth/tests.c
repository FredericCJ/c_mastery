#define _POSIX_C_SOURCE 200809L
#include "recovery.h"
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "MC_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

static void seed(struct ow_owner *o, struct ow_allocator *p)
{
    size_t n[3] = {1, 1, 1};
    CHECK(ow_construct(o, ow_allocator(p), n) == OW_OK);
    for (unsigned i = 0; i < 3; i++)
        o->block[i][0] = (int32_t)(90 + i);
}

static void unchanged(struct ow_owner *o, struct ow_owner old)
{
    for (unsigned i = 0; i < 3; i++)
        CHECK(o->block[i] == old.block[i] &&
              o->block[i][0] == (int32_t)(90 + i));
}

static void admission(void)
{
    const char *cases[] = {"", "3:x", "65536:0", "65535:1", "1:2x", ":1", "1:"};
    const er_code codes[] = {ER_INVALID, ER_INVALID, ER_RANGE,  ER_RANGE,
                             ER_INVALID, ER_INVALID, ER_INVALID};
    struct ow_allocator p = {0};
    struct ow_owner o = {0};
    seed(&o, &p);
    struct ow_owner old = o;
    for (unsigned i = 0; i < 7; i++)
    {
        struct mc_publish r =
            mc_publish_pair((const unsigned char *)cases[i], strlen(cases[i]),
                            ow_allocator(&p), &o);
        CHECK(!r.committed && r.parse.tag == codes[i] && r.storage == OW_OK);
        unchanged(&o, old);
        CHECK(p.attempts == 3 && p.releases == 0 && p.live == 3);
    }
    CHECK(!mc_publish_pair(NULL, 0, ow_allocator(&p), &o).committed);
    CHECK(!mc_publish_pair(NULL, (size_t)INT32_MAX + 1, ow_allocator(&p), &o)
               .committed);
    unchanged(&o, old);
    CHECK(p.attempts == 3 && p.releases == 0);
    CHECK(ow_destroy(&o) == OW_OK && p.live == 0);
    puts("MCP_ADMISSION empty-invalid-range trailing zero-allocations");
}

static void prefixes(void)
{
    for (unsigned f = 0; f < 4; f++)
    {
        struct ow_allocator p = {0};
        struct ow_owner o = {0};
        seed(&o, &p);
        struct ow_owner old = o;
        p.fail_at = f ? 3 + f : 0;
        struct mc_publish r = mc_publish_pair((const unsigned char *)"3:7", 3,
                                              ow_allocator(&p), &o);
        CHECK(r.parse.tag == ER_OK && r.parse.as.value == 10);
        if (f)
        {
            CHECK(!r.committed && r.storage == OW_NOMEM);
            unchanged(&o, old);
            CHECK(p.attempts == 3 + f && p.releases == f - 1 && p.live == 3 &&
                  p.peak == 3 + f - 1);
        }
        else
        {
            CHECK(r.committed && r.storage == OW_OK && p.attempts == 6 &&
                  p.releases == 3 && p.live == 3 && p.peak == 6);
            CHECK(o.block[0][0] == 10 && o.block[1][0] == 2 &&
                  o.block[2][0] == 3);
        }
        CHECK(ow_destroy(&o) == OW_OK && p.live == 0 &&
              p.invalid_releases == 0);
        CHECK(p.releases == (f ? f + 2 : 6));
    }
    puts("MCP_PREFIX actual-parser-allocator-owner all-three-failure-cuts");
}

struct mc_port
{
    int fd[8];
    unsigned live[8], next, opens, fail_open, closes, used, plan[4], order[12];
};

static int open_resource(void *ctx, unsigned ordinal, int *out)
{
    struct mc_port *p = ctx;
    CHECK(ordinal < 2);
    ++p->opens;
    if (p->opens == p->fail_open)
        return 0;
    CHECK(p->next < 8);
    int fd = open("/dev/null", O_RDONLY);
    CHECK(fd >= 0);
    unsigned id = p->next++;
    p->fd[id] = fd;
    p->live[id] = 1;
    *out = (int)id;
    return 1;
}

static enum mc_close close_resource(void *ctx, int id)
{
    struct mc_port *p = ctx;
    CHECK(id >= 0 && (unsigned)id < p->next && p->live[id] && p->closes < 12);
    p->order[p->closes++] = (unsigned)id;
    unsigned r = p->used < 4 ? p->plan[p->used++] : 0;
    if (r != 1)
    {
        CHECK(close(p->fd[id]) == 0);
        p->live[id] = 0;
    }
    return (enum mc_close)r;
}

static unsigned live(const struct mc_port *p)
{
    unsigned n = 0;
    for (unsigned i = 0; i < p->next; i++)
        n += p->live[i];
    return n;
}

static void finish(struct mc_recovery *s, struct ow_allocator *a,
                   struct mc_port *p)
{
    p->used = 4;
    unsigned errors = 99;
    if (s->recovering)
        CHECK(mc_recover(s, s->generation, &errors));
    uint32_t token = 0;
    CHECK(mc_retire_current(s, &token));
    CHECK(mc_recover(s, token, &errors) && errors == 0);
    CHECK(!live(p) && !a->live && !a->invalid_releases);
}

static void close_case(unsigned code, int output)
{
    struct mc_port p = {0};
    struct ow_allocator a = {0};
    struct mc_recovery s;
    CHECK(mc_recovery_init(
        &s, (struct mc_resources){&p, open_resource, close_resource}));
    CHECK(mc_publish_recoverable(&s, (const unsigned char *)"2:3", 3,
                                 ow_allocator(&a))
              .publication.committed);
    unsigned k = code;
    for (unsigned i = 0; i < 4; i++)
    {
        p.plan[i] = k % 3;
        k /= 3;
    }
    struct mc_recovered r = mc_publish_recoverable(
        &s, (const unsigned char *)"7:9", 3, ow_allocator(&a));
    CHECK(r.publication.committed && s.current.cells.block[0][0] == 16);
    CHECK(p.order[0] == 1 && p.order[1] == 0);
    unsigned first_mask = s.pending.mask, first_errors = r.close_errors,
             first_pending = s.recovering, first_cells = (unsigned)a.live,
             first_resources = live(&p);
    CHECK(first_mask == (p.live[0] | p.live[1] << 1));
    unsigned second_errors = 99;
    int done = mc_recover(&s, r.recovery_token, &second_errors);
    CHECK(s.pending.mask == (p.live[0] | p.live[1] << 1));
    CHECK((a.live == 6) == (s.pending.mask != 0));
    if (output)
        printf("%u,%u,%u,%u,%u,%u,%d,%u,%u,%u,%zu,%u,%u\n", code, first_mask,
               first_errors, first_pending, first_cells, first_resources, done,
               second_errors, s.pending.mask, s.recovering, a.live, live(&p),
               p.closes);
    finish(&s, &a, &p);
}

static void close_order(void)
{
    for (unsigned i = 0; i < 81; i++)
        close_case(i, 0);
    puts("MCP_CLOSE 81-outcome-words retained-versus-released-error real-fds");
}

static void recovery(void)
{
    for (unsigned f = 1; f <= 2; f++)
    {
        struct mc_port p = {0};
        struct ow_allocator a = {0};
        struct mc_recovery s;
        CHECK(mc_recovery_init(
            &s, (struct mc_resources){&p, open_resource, close_resource}));
        CHECK(mc_publish_recoverable(&s, (const unsigned char *)"2:3", 3,
                                     ow_allocator(&a))
                  .publication.committed);
        struct mc_five old = s.current;
        p.fail_open = p.opens + f;
        for (unsigned i = 0; i < 4; i++)
            p.plan[i] = 1;
        struct mc_recovered r = mc_publish_recoverable(
            &s, (const unsigned char *)"7:9", 3, ow_allocator(&a));
        CHECK(!r.publication.committed && r.publication.parse.tag == ER_OK &&
              r.publication.storage == OW_NOMEM);
        for (unsigned i = 0; i < 3; i++)
            CHECK(s.current.cells.block[i] == old.cells.block[i]);
        CHECK(s.current.cells.block[0][0] == 5);
        CHECK((s.recovering != 0) == (f == 2));
        CHECK(a.live == (f == 2 ? 6u : 3u));
        CHECK(live(&p) == (f == 2 ? 3u : 2u));
        if (f == 2)
        {
            unsigned errors = 99;
            size_t attempts = a.attempts;
            unsigned opens = p.opens, closes = p.closes;
            CHECK(!mc_recover(&s, s.generation - 1, &errors) && errors == 99);
            CHECK(!mc_publish_recoverable(&s, (const unsigned char *)"1:1", 3,
                                          ow_allocator(&a))
                       .publication.committed);
            CHECK(a.attempts == attempts && p.opens == opens &&
                  p.closes == closes);
            CHECK(!mc_recover(&s, s.generation, &errors) && errors == 1);
        }
        finish(&s, &a, &p);
    }
    puts(
        "MCP_RECOVERY precommit-old-owner retained-stage stale-token blocked-publish");
}

static void limits(void)
{
    struct mc_port p = {0};
    struct ow_allocator a = {0};
    struct mc_recovery s;
    CHECK(mc_recovery_init(
        &s, (struct mc_resources){&p, open_resource, close_resource}));
    CHECK(mc_publish_recoverable(&s, (const unsigned char *)"2:3", 3,
                                 ow_allocator(&a))
              .publication.committed);
    s.generation = UINT32_MAX - 2;
    CHECK(mc_publish_recoverable(&s, (const unsigned char *)"7:9", 3,
                                 ow_allocator(&a))
              .publication.committed &&
          s.generation == UINT32_MAX - 1);
    size_t attempts = a.attempts;
    unsigned opens = p.opens;
    CHECK(!mc_publish_recoverable(&s, (const unsigned char *)"2:3", 3,
                                  ow_allocator(&a))
               .publication.committed);
    CHECK(a.attempts == attempts && p.opens == opens);
    finish(&s, &a, &p);
    uint32_t token = 91;
    CHECK(s.generation == UINT32_MAX && !mc_retire_current(&s, &token) &&
          token == 91);
    unsigned errors = 99;
    CHECK(!mc_recover(&s, UINT32_MAX, &errors) && errors == 99);
    puts("MCP_LIMIT final-publication reserves-final-retirement no-wrap");
}

static void metrics(void)
{
    double times[7];
    uint64_t checksum = 0;
    for (unsigned j = 0; j < 7; j++)
    {
        clock_t start = clock();
        struct ow_allocator a = {0};
        struct ow_owner o = {0};
        for (unsigned i = 0; i < 10000; i++)
        {
            struct mc_publish r = mc_publish_pair((const unsigned char *)"3:7",
                                                  3, ow_allocator(&a), &o);
            CHECK(r.committed);
            checksum += (unsigned)o.block[0][0];
        }
        CHECK(ow_destroy(&o) == OW_OK && a.live == 0 && a.attempts == 30000 &&
              a.releases == 30000);
        times[j] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf(
        "{\"iterations\":10000,\"checksum\":%" PRIu64
        ",\"attempts_per_sample\":30000,\"releases_per_sample\":30000,\"recovery_size\":%zu,\"seconds\":[",
        checksum, sizeof(struct mc_recovery));
    for (unsigned i = 0; i < 7; i++)
        printf("%s%.9f", i ? "," : "", times[i]);
    puts("]}");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "publication_admission"))
        admission();
    else if (!strcmp(argv[1], "publication_prefix"))
        prefixes();
    else if (!strcmp(argv[1], "publication_close"))
        close_order();
    else if (!strcmp(argv[1], "publication_recovery"))
        recovery();
    else if (!strcmp(argv[1], "publication_limits"))
        limits();
    else if (!strcmp(argv[1], "oracle"))
    {
        for (unsigned i = 0; i < 81; i++)
            close_case(i, 1);
    }
    else if (!strcmp(argv[1], "metrics"))
        metrics();
    else
        CHECK(0);
    return 0;
}
