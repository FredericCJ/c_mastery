#include "cow_resources.h"
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "OW_CHECK OCR line%d: %s\n", __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)
/* Emergency process-only cleanup prevents a semantic assertion from being
 * replaced by a leak-detector exit. Successful cases assert real release counts. */
static void *allocation[64];

static void emergency(void)
{
    for (unsigned i = 0; i < 64; ++i)
        free(allocation[i]);
}

struct ow_raw
{
    unsigned calls, releases, live, fail;
};

static void *take(void *p, size_t n)
{
    struct ow_raw *r = p;
    ++r->calls;
    if (r->calls == r->fail)
        return NULL;
    for (unsigned i = 0; i < 64; ++i)
        if (!allocation[i])
        {
            void *b = malloc(n);
            if (b)
            {
                allocation[i] = b;
                ++r->live;
            }
            return b;
        }
    return NULL;
}

static void release(void *p, void *b)
{
    struct ow_raw *r = p;
    for (unsigned i = 0; i < 64; ++i)
        if (allocation[i] == b)
        {
            allocation[i] = NULL;
            break;
        }
    --r->live;
    ++r->releases;
    free(b);
}

struct ow_resources
{
    struct ow_allocator cells;
    unsigned clones, drops, fail_after;
};

static enum ow_status clone(void *p, const struct ocr_resource *from,
                            struct ocr_resource *to)
{
    struct ow_resources *r = p;
    ++r->clones;
    struct v4_allocator a = ow_allocator(&r->cells);
    int32_t *b = a.take(a.context, from->length);
    if (!b)
        return OW_NOMEM;
    *to = (struct ocr_resource){b, from->length};
    for (size_t i = 0; i < from->length; ++i)
        b[i] = from->data[i];
    return r->clones == r->fail_after ? OW_NOMEM : OW_OK;
}

static void drop(void *p, struct ocr_resource *v)
{
    struct ow_resources *r = p;
    if (v->data)
    {
        struct v4_allocator a = ow_allocator(&r->cells);
        a.release(a.context, v->data);
        ++r->drops;
        *v = (struct ocr_resource){0};
    }
}

static enum ow_status setup(struct ocr_store *s, struct ow_raw *raw,
                            struct ow_resources *r)
{
    int32_t a[2] = {1, 2}, b[2] = {3, 4}, c[2] = {5, 6};
    struct ocr_resource seed[3] = {{a, 2}, {b, 2}, {c, 2}};
    return ocr_init(s, (struct ocr_memory){raw, take, release},
                    (struct ocr_ops){r, clone, drop}, seed);
}

static int baseline(void)
{
    struct ow_allocator pool = {0};
    struct v4_allocator a = ow_allocator(&pool);
    int32_t *data = a.take(a.context, 3);
    CHECK(data);
    for (unsigned i = 0; i < 3; ++i)
        data[i] = (int32_t)(i + 1);
    struct ow_cow old = {1, data, 3, a}, fresh = {0}, *owner = &old;
    CHECK(
        ow_cow_clone(&old) ==
        OW_OK); /* Defined single-threaded raw alias deliberately bypasses the COW API. */
    data[1] = 44;
    CHECK(owner->data[1] == 44);
    data[1] = 2;
    pool.fail_at = 2;
    CHECK(ow_cow_write(&owner, &fresh, 1, 9) == OW_NOMEM && owner == &old &&
          old.refs == 2 && !fresh.refs && data[1] == 2);
    pool.fail_at = 0;
    CHECK(ow_cow_write(&owner, &fresh, 1, 9) == OW_OK && owner == &fresh &&
          old.refs == 1 && data[1] == 2 && fresh.data[1] == 9);
    size_t before = pool.attempts;
    CHECK(ow_cow_write(&owner, NULL, 0, 8) == OW_OK && pool.attempts == before);
    old.refs = UINT_MAX;
    CHECK(ow_cow_clone(&old) == OW_FULL && old.refs == UINT_MAX);
    old.refs = 1;
    CHECK(ow_cow_drop(&old) == OW_OK && ow_cow_drop(&fresh) == OW_OK &&
          !pool.live);
    puts(
        "OCR_BASE actual-OwCow unique-shared fail-before-publication maximum-refusal raw-alias-bypass");
    return 0;
}

static int snapshots(void)
{
    struct ow_raw raw = {0};
    struct ow_resources r = {0};
    struct ocr_store s = {0};
    CHECK(setup(&s, &raw, &r) == OW_OK && raw.live == 1 && r.cells.live == 3);
    CHECK(ocr_write(&s, 0, 0, 7) == OW_OK && r.clones == 3);
    struct ocr_snapshot first, second;
    CHECK(ocr_snapshot(&s, &first) == OW_OK);
    CHECK(ocr_write(&s, 0, 0, 9) == OW_OK && r.clones == 6 && raw.live == 2);
    CHECK(ocr_snapshot(&s, &second) == OW_OK);
    int32_t v = 99;
    CHECK(ocr_read(first, 0, 0, &v) == OW_OK && v == 7);
    CHECK(ocr_read(second, 0, 0, &v) == OW_OK && v == 9);
    CHECK(ocr_read(first, 3, 0, &v) == OW_INVALID && v == 9);
    CHECK(ocr_release(first) == OW_OK && raw.live == 1 && r.cells.live == 3);
    CHECK(ocr_release(second) == OW_OK);
    unsigned before = r.clones;
    CHECK(ocr_write(&s, 1, 1, 11) == OW_OK && r.clones == before);
    CHECK(ocr_destroy(&s) == OW_OK && !raw.live && !r.cells.live &&
          r.cells.releases == 6);
    puts(
        "OCR_SNAPSHOTS actual-owned-resources immutable-old unique-no-clone detached-reclamation");
    return 0;
}

static int failures(void)
{
    for (unsigned kind = 0; kind < 3; ++kind)
        for (unsigned point = 1; point <= 3; ++point)
        {
            struct ow_raw raw = {0};
            struct ow_resources r = {0};
            struct ocr_store s = {0};
            CHECK(setup(&s, &raw, &r) == OW_OK);
            struct ocr_snapshot old;
            CHECK(ocr_snapshot(&s, &old) == OW_OK);
            unsigned releases = r.drops;
            if (kind == 0)
                raw.fail = raw.calls + 1;
            else if (kind == 1)
                r.cells.fail_at = r.cells.attempts + point;
            else
                r.fail_after = r.clones + point;
            CHECK(ocr_write(&s, 0, 0, 77) == OW_NOMEM && raw.live == 1 &&
                  r.cells.live == 3);
            CHECK(r.drops == releases + (kind == 0   ? 0
                                         : kind == 1 ? point - 1
                                                     : point));
            int32_t v = 0;
            CHECK(ocr_read(old, 0, 0, &v) == OW_OK && v == 1);
            struct ocr_snapshot current;
            CHECK(ocr_snapshot(&s, &current) == OW_OK &&
                  ocr_read(current, 0, 0, &v) == OW_OK && v == 1);
            CHECK(ocr_release(current) == OW_OK && ocr_release(old) == OW_OK);
            raw.fail = 0;
            r.cells.fail_at = 0;
            r.fail_after = 0;
            CHECK(ocr_destroy(&s) == OW_OK && !raw.live && !r.cells.live);
        }
    for (unsigned point = 1; point <= 3; ++point)
    {
        struct ow_raw raw = {0};
        struct ow_resources r = {0};
        struct ocr_store s = {0};
        r.fail_after = point;
        CHECK(setup(&s, &raw, &r) == OW_NOMEM && !s.ready && !raw.live &&
              !r.cells.live && r.drops == point);
    }
    puts(
        "OCR_FAILURES raw typed post-acquisition-partial-clone reverse-frontier unchanged-owners");
    return 0;
}

static int identity(void)
{
    struct ow_raw raw = {0};
    struct ow_resources r = {0};
    struct ocr_store s = {0};
    CHECK(setup(&s, &raw, &r) == OW_OK);
    struct ocr_snapshot slots[4], unchanged = {NULL, 91, 92};
    for (unsigned i = 0; i < 4; ++i)
        CHECK(ocr_snapshot(&s, &slots[i]) == OW_OK);
    CHECK(ocr_snapshot(&s, &unchanged) == OW_FULL && unchanged.slot == 91);
    struct ocr_snapshot stale = slots[0];
    CHECK(ocr_release(slots[0]) == OW_OK &&
          ocr_snapshot(&s, &slots[0]) == OW_OK);
    int32_t v = 123;
    CHECK(ocr_read(stale, 0, 0, &v) == OW_STALE && v == 123 &&
          ocr_release(stale) == OW_STALE);
    for (unsigned i = 0; i < 4; ++i)
        CHECK(ocr_release(slots[i]) == OW_OK);
    s.loan[0].serial = UINT_MAX - 1;
    for (unsigned i = 1; i < 4; ++i)
        s.loan[i].retired = 1;
    CHECK(ocr_snapshot(&s, &slots[0]) == OW_OK && slots[0].serial == UINT_MAX);
    CHECK(ocr_release(slots[0]) == OW_OK && s.loan[0].retired);
    CHECK(ocr_snapshot(&s, &unchanged) == OW_FULL && unchanged.slot == 91);
    CHECK(ocr_destroy(&s) == OW_OK && !raw.live && !r.cells.live);
    puts(
        "OCR_IDENTITY four-slot-capacity stale-generation final-serial-release-retirement");
    return 0;
}

static int close_case(void)
{
    struct ow_raw raw = {0};
    struct ow_resources r = {0};
    struct ocr_store s = {0};
    CHECK(setup(&s, &raw, &r) == OW_OK);
    struct ocr_snapshot old, out = {NULL, 91, 92};
    CHECK(ocr_snapshot(&s, &old) == OW_OK && ocr_close(&s) == OW_OK);
    CHECK(raw.live == 1 && r.cells.live == 3 &&
          ocr_snapshot(&s, &out) == OW_DEAD && out.slot == 91 &&
          ocr_write(&s, 0, 0, 77) == OW_DEAD);
    CHECK(ocr_destroy(&s) == OW_BUSY);
    int32_t v = 0;
    CHECK(ocr_read(old, 2, 1, &v) == OW_OK && v == 6);
    CHECK(ocr_release(old) == OW_OK && !raw.live && !r.cells.live);
    CHECK(ocr_destroy(&s) == OW_OK);
    puts("OCR_CLOSE no-new-snapshot retained-old-read release-before-destroy");
    return 0;
}

struct ow_worker
{
    struct ocr_store *store;
    unsigned bad, success, refused;
};

static void *reader(void *p)
{
    struct ow_worker *w = p;
    for (unsigned i = 0; i < 400; ++i)
    {
        struct ocr_snapshot snapshot;
        if (ocr_snapshot(w->store, &snapshot) != OW_OK)
        {
            w->bad = 1;
            return NULL;
        }
        int32_t a = 0, b = 0;
        if (ocr_read(snapshot, 0, 0, &a) != OW_OK)
            w->bad = 1;
        sched_yield();
        if (ocr_read(snapshot, 0, 0, &b) != OW_OK || a != b)
            w->bad = 1;
        if (ocr_release(snapshot) != OW_OK)
            w->bad = 1;
        ++w->success;
    }
    return NULL;
}

static void *writer(void *p)
{
    struct ow_worker *w = p;
    for (unsigned i = 0; i < 500; ++i)
    {
        enum ow_status q = ocr_write(w->store, 0, 0, (int32_t)(i + 100));
        if (q == OW_OK)
            ++w->success;
        else if (q == OW_NOMEM)
            ++w->refused;
        else
            w->bad = 1;
        sched_yield();
    }
    return NULL;
}

static int threads(void)
{
    struct ow_raw raw = {0};
    struct ow_resources r = {0};
    struct ocr_store s = {0};
    CHECK(setup(&s, &raw, &r) == OW_OK);
    struct ocr_snapshot old;
    CHECK(ocr_snapshot(&s, &old) == OW_OK && ocr_write(&s, 0, 0, 9) == OW_OK);
    pthread_t t[3];
    struct ow_worker w[3] = {{&s, 0, 0, 0}, {&s, 0, 0, 0}, {&s, 0, 0, 0}};
    CHECK(pthread_create(&t[0], NULL, reader, &w[0]) == 0 &&
          pthread_create(&t[1], NULL, reader, &w[1]) == 0 &&
          pthread_create(&t[2], NULL, writer, &w[2]) == 0);
    for (unsigned i = 0; i < 3; ++i)
        CHECK(pthread_join(t[i], NULL) == 0 && !w[i].bad);
    CHECK(w[0].success + w[1].success == 800 &&
          w[2].success + w[2].refused == 500);
    int32_t value = 0;
    CHECK(ocr_read(old, 0, 0, &value) == OW_OK && value == 1);
    CHECK(ocr_release(old) == OW_OK && ocr_write(&s, 0, 0, 777) == OW_OK &&
          ocr_destroy(&s) == OW_OK && !raw.live && !r.cells.live);
    printf(
        "OCR_THREADS actual-two-readers one-writer reads800 attempts500 accepted%u refused%u old-version-retained joined\n",
        w[2].success, w[2].refused);
    return 0;
}

static int metrics(void)
{
    struct ow_raw raw = {0};
    struct ow_resources r = {0};
    struct ocr_store s = {0};
    CHECK(setup(&s, &raw, &r) == OW_OK);
    double times[7];
    unsigned sum = 0;
    for (unsigned k = 0; k < 7; ++k)
    {
        clock_t start = clock();
        for (unsigned i = 0; i < 1000; ++i)
        {
            CHECK(ocr_write(&s, 0, 0, (int32_t)i) == OW_OK);
            sum += i;
        }
        times[k] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    CHECK(r.clones == 3);
    struct ocr_snapshot old;
    CHECK(ocr_snapshot(&s, &old) == OW_OK && ocr_write(&s, 0, 0, 5) == OW_OK &&
          r.clones == 6);
    CHECK(ocr_release(old) == OW_OK && ocr_destroy(&s) == OW_OK &&
          !r.cells.live && !raw.live);
    printf(
        "{\"iterations\":1000,\"checksum\":%u,\"unique_clone_calls\":0,\"detach_clone_calls\":3,\"store_size\":%zu,\"token_size\":%zu,\"seconds\":[",
        sum, sizeof(struct ocr_store), sizeof(struct ocr_snapshot));
    for (unsigned i = 0; i < 7; ++i)
        printf("%s%.9f", i ? "," : "", times[i]);
    puts("]}");
    return 0;
}

/* Scalar retained producer driver: each line is an independent legal trace. */
static int oracle(void)
{
    char text[16];
    while (scanf("%15s", text) == 1)
    {
        struct ow_allocator p = {0};
        struct v4_allocator a = ow_allocator(&p);
        struct ow_cow block[16] = {{0}}, *owner[3] = {NULL, NULL, NULL};
        int32_t *data = a.take(a.context, 2);
        if (!data)
            return 2;
        data[0] = 1;
        data[1] = 2;
        block[0] = (struct ow_cow){1, data, 2, a};
        owner[0] = &block[0];
        unsigned used = 1;
        for (unsigned j = 0; text[j] && text[j] != '-'; ++j)
        {
            unsigned op = (unsigned)(text[j] - 'a');
            enum ow_status q = OW_INVALID;
            if (op < 3)
            {
                unsigned from = op, to = (op + 1) % 3;
                if (owner[from] && !owner[to])
                {
                    q = ow_cow_clone(owner[from]);
                    if (q == OW_OK)
                        owner[to] = owner[from];
                }
            }
            else if (op < 6)
            {
                unsigned i = op - 3;
                if (owner[i])
                {
                    q = ow_cow_drop(owner[i]);
                    if (q == OW_OK)
                        owner[i] = NULL;
                }
            }
            else if (op < 12)
            {
                unsigned i = (op - 6) % 3;
                if (owner[i])
                {
                    p.fail_at = op >= 9 ? p.attempts + 1 : 0;
                    q = ow_cow_write(&owner[i], &block[used], 0, 7);
                    if (owner[i] == &block[used])
                        ++used;
                    p.fail_at = 0;
                }
            }
            else
                return 2;
            printf("%d:", q);
            for (unsigned i = 0; i < 3; ++i)
                if (owner[i])
                    printf("%d,%d;", owner[i]->data[0], owner[i]->data[1]);
                else
                    printf("-;");
            printf("%zu,%zu|", p.live, p.attempts);
        }
        for (unsigned i = 0; i < 3; ++i)
            if (owner[i] && ow_cow_drop(owner[i]) != OW_OK)
                return 2;
        if (p.live)
            return 2;
        puts("");
    }
    return ferror(stdin) ? 2 : 0;
}

int main(int argc, char **argv)
{
    if (atexit(emergency) != 0 || argc != 2)
        return 2;
    if (!strcmp(argv[1], "cow_resources_base"))
        return baseline();
    if (!strcmp(argv[1], "cow_resources_snapshots"))
        return snapshots();
    if (!strcmp(argv[1], "cow_resources_failures"))
        return failures();
    if (!strcmp(argv[1], "cow_resources_identity"))
        return identity();
    if (!strcmp(argv[1], "cow_resources_close"))
        return close_case();
    if (!strcmp(argv[1], "cow_resources_threads"))
        return threads();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    return 2;
}
