#include "foundation.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "FAIL %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)
#define S(e, w)                                                             \
    do                                                                      \
    {                                                                       \
        enum f_status status = (e);                                         \
        if (status != (w))                                                  \
        {                                                                   \
            fprintf(stderr, "FAIL %s:%d %s got %s expected %s\n", __FILE__, \
                    __LINE__, #e, f_status_name(status), f_status_name(w)); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

static int prefix(void)
{
    int32_t a[] = {3, -1, 4}, out[] = {99, 99, 99};
    S(f_prefix(a, 3, out, 3), F_OK);
    CHECK(out[0] == 3 && out[1] == 2 && out[2] == 6);
    int32_t fail[] = {INT32_MAX, 1, -1};
    out[0] = out[1] = out[2] = 99;
    S(f_prefix(fail, 3, out, 3), F_RANGE);
    CHECK(out[0] == 99 && out[1] == 99 && out[2] == 99);
    S(f_prefix(a, 3, out, 2), F_FULL);
    S(f_prefix(0, 0, 0, 0), F_OK);
    S(f_prefix(a, 3, a, 3), F_OK);
    CHECK(a[2] == 6);
    return 0;
}

static int size(void)
{
    size_t out = 77;
    S(f_checked_size(8, 4, 1, &out), F_OK);
    CHECK(out == 33);
    out = 77;
    S(f_checked_size(SIZE_MAX, 2, 1, &out), F_RANGE);
    CHECK(out == 77);
    S(f_checked_size(SIZE_MAX, 0, 5, &out), F_OK);
    CHECK(out == 5);
    S(f_checked_size(0, 0, SIZE_MAX, &out), F_OK);
    CHECK(out == SIZE_MAX);
    return 0;
}

static int cursor(void)
{
    const uint8_t bytes[] = {1, 0, 3};
    const uint8_t *p = 0;
    struct f_cursor c = {bytes, 3, 0};
    S(f_take(&c, 2, &p), F_OK);
    CHECK(p == bytes && c.position == 2);
    S(f_take(&c, 2, &p), F_RANGE);
    CHECK(c.position == 2);
    S(f_take(&c, 1, &p), F_OK);
    CHECK(*p == 3);
    S(f_take(&c, 0, &p), F_OK);
    CHECK(p == bytes + 3);
    struct f_cursor empty = {0, 0, 0};
    S(f_take(&empty, 0, &p), F_OK);
    CHECK(!p);
    return 0;
}

static int owner(void)
{
    struct f_buffer b = {{0}, 0, 0};
    const uint8_t bytes[] = {1, 0, 3};
    struct f_view v;
    uint8_t out = 0;
    S(f_append(&b, bytes, 3), F_OK);
    S(f_view(&b, 1, 2, &v), F_OK);
    S(f_view_read(v, 0, &out), F_OK);
    CHECK(out == 0);
    S(f_append(&b, b.data, 3), F_OK);
    CHECK(b.length == 6 && b.data[3] == 1 && b.data[5] == 3);
    S(f_view_read(v, 0, &out), F_STALE);
    size_t before = b.length;
    S(f_append(&b, bytes, 30), F_FULL);
    CHECK(b.length == before);
    return 0;
}

static int parse(void)
{
    int32_t result = 77;
    const char *valid[] = {"0", "-0", "2147483647", "-2147483648", "0012"};
    const int32_t expected[] = {0, 0, INT32_MAX, INT32_MIN, 12};
    for (unsigned i = 0; i < 5; i++)
    {
        S(f_parse_i32((const uint8_t *)valid[i], strlen(valid[i]), &result),
          F_OK);
        CHECK(result == expected[i]);
    }
    const char *invalid[] = {"", "-", "+1", " 1", "1x", "1\n"};
    for (unsigned i = 0; i < 6; i++)
    {
        result = 77;
        S(f_parse_i32((const uint8_t *)invalid[i], strlen(invalid[i]), &result),
          F_INVALID);
        CHECK(result == 77);
    }
    result = 77;
    S(f_parse_i32((const uint8_t *)"2147483648", 10, &result), F_RANGE);
    CHECK(result == 77);
    return 0;
}

static int sort(void)
{
    struct f_record a[] = {{4, 0}, {2, 1}, {5, 2}, {2, 3}, {1, 4}, {4, 5}};
    struct f_work w;
    S(f_sort(a, 6, &w), F_OK);
    CHECK(w.comparisons == 11 && w.shifts == 8 && w.placements == 5);
    CHECK(a[1].ordinal == 1 && a[2].ordinal == 3 && a[3].ordinal == 0 &&
          a[4].ordinal == 5);
    return 0;
}

static uint32_t rng(uint32_t *state)
{
    *state = *state * UINT32_C(1664525) + UINT32_C(1013904223);
    return *state;
}

static int properties(void)
{
    uint32_t seed = 0x13579bdu;
    for (unsigned round = 0; round < 256; round++)
    {
        struct f_record a[32];
        unsigned n = rng(&seed) % 33;
        uint64_t inversions = 0;
        struct f_work work;
        for (unsigned i = 0; i < n; i++)
        {
            a[i].key = (int32_t)(rng(&seed) % 7);
            a[i].ordinal = i;
        }
        for (unsigned i = 0; i < n; i++)
            for (unsigned j = i + 1; j < n; j++)
                inversions += a[i].key > a[j].key;
        S(f_sort(a, n, &work), F_OK);
        CHECK(work.shifts == inversions && work.placements == (n ? n - 1 : 0));
        for (unsigned i = 1; i < n; i++)
            CHECK(a[i - 1].key < a[i].key || (a[i - 1].key == a[i].key &&
                                              a[i - 1].ordinal < a[i].ordinal));
        S(f_sort(a, n, &work), F_OK);
        CHECK(work.shifts == 0);
    }
    return 0;
}

struct f_script
{
    ptrdiff_t steps[8];
    unsigned i;
    uint8_t output[32];
    size_t n;
};

static ptrdiff_t writer(void *ctx, const uint8_t *p, size_t n)
{
    struct f_script *s = ctx;
    ptrdiff_t step = s->steps[s->i++];
    if (step > 0 && (size_t)step <= n)
    {
        memcpy(s->output + s->n, p, (size_t)step);
        s->n += (size_t)step;
    }
    return step;
}

static int io(void)
{
    const uint8_t b[] = {1, 2, 3, 4, 5};
    size_t accepted;
    struct f_script s = {{2, -2, 1, 2}, 0, {0}, 0};
    S(f_write_all(writer, &s, b, 5, 4, &accepted), F_OK);
    CHECK(accepted == 5 && !memcmp(s.output, b, 5));
    s = (struct f_script){{2, 0}, 0, {0}, 0};
    S(f_write_all(writer, &s, b, 5, 2, &accepted), F_IO);
    CHECK(accepted == 2);
    s = (struct f_script){{-2, -2}, 0, {0}, 0};
    S(f_write_all(writer, &s, b, 5, 2, &accepted), F_PENDING);
    CHECK(accepted == 0);
    s = (struct f_script){{6}, 0, {0}, 0};
    S(f_write_all(writer, &s, b, 5, 1, &accepted), F_IO);
    CHECK(accepted == 0);
    return 0;
}

static int async(void)
{
    struct f_async a = {F_IDLE, 0, 0};
    uint32_t old, new_ticket;
    S(f_submit(&a, &old), F_OK);
    S(f_cancel(&a, old), F_PENDING);
    CHECK(a.releases == 0 && a.state == F_CANCEL_REQUESTED);
    S(f_submit(&a, &new_ticket), F_PENDING);
    S(f_complete(&a, old), F_OK);
    CHECK(a.releases == 1);
    S(f_complete(&a, old), F_INVALID);
    S(f_submit(&a, &new_ticket), F_OK);
    CHECK(new_ticket != old);
    S(f_complete(&a, old), F_STALE);
    CHECK(a.releases == 1 && a.state == F_INFLIGHT);
    S(f_complete(&a, new_ticket), F_OK);
    CHECK(a.releases == 2);
    return 0;
}

struct f_acquisitions
{
    unsigned fail, live, released[3], count;
};

static int acquire(void *context, unsigned id)
{
    struct f_acquisitions *a = context;
    if (id == a->fail)
        return -1;
    a->live |= 1u << id;
    return 0;
}

static void release(void *context, unsigned id)
{
    struct f_acquisitions *a = context;
    a->released[a->count++] = id;
    a->live &= ~(1u << id);
}

static int cleanup(void)
{
    for (unsigned fail = 0; fail < 3; fail++)
    {
        struct f_acquisitions a = {fail, 0, {0}, 0};
        struct f_resources r = {&a, acquire, release};
        unsigned owned = 99;
        S(f_construct(r, &owned), F_IO);
        CHECK(!a.live && owned == 99 && a.count == fail);
        for (unsigned i = 0; i < fail; i++)
            CHECK(a.released[i] == fail - i - 1);
    }
    struct f_acquisitions a = {3, 0, {0}, 0};
    struct f_resources r = {&a, acquire, release};
    unsigned owned = 0;
    S(f_construct(r, &owned), F_OK);
    CHECK(owned == 7 && a.live == 7);
    for (unsigned i = 3; i > 0; i--)
        release(&a, i - 1);
    CHECK(!a.live);
    return 0;
}

static int packet(void)
{
    uint16_t out[4] = {99, 99, 99, 99};
    size_t count = 99;
    const uint8_t valid[] = {1, 2, 0, 1, 0, 2};
    S(f_packet(valid, sizeof valid, 0, out, &count), F_OK);
    CHECK(count == 2 && out[0] == 1 && out[1] == 2);
    out[0] = 99;
    count = 99;
    S(f_packet(valid, 4, 0, out, &count), F_INVALID);
    CHECK(count == 99 && out[0] == 99);
    const uint8_t denied[] = {2, 0};
    S(f_packet(denied, 2, 0, out, &count), F_DENIED);
    CHECK(count == 99);
    S(f_packet(denied, 2, 1, out, &count), F_OK);
    CHECK(!count);
    const uint8_t excessive[] = {1, 5};
    S(f_packet(excessive, 2, 0, out, &count), F_RANGE);
    return 0;
}

int main(int argc, char **argv)
{
    struct
    {
        const char *name;
        int (*run)(void);
    } cases[] = {
        {"prefix", prefix},         {"size", size},    {"cursor", cursor},
        {"owner", owner},           {"parse", parse},  {"sort", sort},
        {"properties", properties}, {"io", io},        {"async", async},
        {"cleanup", cleanup},       {"packet", packet}};

    if (argc != 2)
        return 2;
    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++)
        if (!strcmp(cases[i].name, argv[1]))
            return cases[i].run();
    return 2;
}
