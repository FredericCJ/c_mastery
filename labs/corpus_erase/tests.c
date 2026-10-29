#include "erase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "ER_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(41);                                           \
        }                                                       \
    } while (0)

struct er_context
{
    unsigned mask, calls, fail, drops[16], count;
};

static int predicate(void *p, const struct er_item *i)
{
    struct er_context *c = p;
    if (++c->calls == c->fail)
        return -1;
    return (int)((c->mask >> (i->id - 1)) & 1);
}

static void drop(void *p, unsigned id)
{
    struct er_context *c = p;
    CHECK(id < 16 && id && c->drops[id] == 0);
    ++c->drops[id];
    ++c->count;
}

static int threshold(void *p, const struct er_item *i)
{
    return i->value == *(const int *)p;
}

int main(void)
{
    int a[] = {1, 2, 1, 3};
    size_t end = 99, moves = 99;
    CHECK(er_compact(a, 4, 1, &end, &moves) == ER_OK && end == 2 &&
          moves == 2 && a[0] == 2 && a[1] == 3);
    CHECK(er_compact(NULL, 0, 1, &end, &moves) == ER_OK && end == 0 &&
          moves == 0);
    for (unsigned n = 0; n <= 8; ++n)
        for (unsigned mask = 0; mask < (1u << n); ++mask)
        {
            struct er_item input[12];
            for (unsigned i = 0; i < n; ++i)
                input[i] = (struct er_item){i + 1, (int)i};
            struct er_owner o;
            CHECK(er_init(&o, input, n) == ER_OK);
            struct er_context c = {.mask = mask};
            struct er_tail tail = {0};
            CHECK(er_remove_if(&o, predicate, &c, &tail) == ER_OK &&
                  o.count == n && c.calls == n && !c.count);
            size_t kept = 0;
            for (unsigned i = 0; i < n; ++i)
                if (!(mask >> i & 1))
                {
                    CHECK(o.items[kept].id == i + 1);
                    ++kept;
                }
            CHECK(tail.new_end == kept);
            CHECK(er_erase(&o, tail, drop, &c) == ER_OK && o.count == kept &&
                  c.count == n - kept);
            for (unsigned i = 0; i < n; ++i)
                CHECK(c.drops[i + 1] == (mask >> i & 1));
            CHECK(er_erase(&o, tail, drop, &c) == ER_STALE);
            CHECK(er_clear(&o, drop, &c) == ER_OK && c.count == n);
        }
    struct er_item data[] = {{1, 1}, {2, 2}, {3, 1}, {4, 3}};
    struct er_owner o;
    CHECK(er_init(&o, data, 4) == ER_OK);
    for (unsigned fail = 1; fail <= 4; ++fail)
    {
        struct er_owner before = o;
        struct er_context c = {.mask = 5, .fail = fail};
        struct er_tail token = {0};
        CHECK(er_remove_if(&o, predicate, &c, &token) == ER_FAILED &&
              !memcmp(&o, &before, sizeof o) && token.owner == NULL &&
              c.calls == fail && !c.count);
    }
    struct er_tail first;
    CHECK(er_remove_if(&o, threshold, &o.items[0].value, &first) == ER_OK &&
          first.new_end == 2 && o.items[0].id == 2 && o.items[1].id == 4);
    struct er_context c = {0};
    CHECK(er_erase(&o, first, drop, &c) == ER_OK && c.drops[1] && c.drops[3]);
    struct er_context next = {0};
    struct er_tail second;
    CHECK(er_remove_if(&o, predicate, &next, &second) == ER_OK);
    struct er_owner before = o;
    CHECK(er_erase(&o, first, drop, &c) == ER_STALE &&
          !memcmp(&o, &before, sizeof o));
    CHECK(er_clear(&o, drop, &c) == ER_OK && c.count == 4);
    CHECK(er_erase(&o, second, drop, &c) == ER_STALE);
    CHECK(er_init(&o, data, 4) == ER_OK);
    o.epoch = UINT32_MAX - 1;
    before = o;
    CHECK(er_remove_if(&o, predicate, &next, &second) == ER_EXHAUSTED &&
          !memcmp(&o, &before, sizeof o));
    c = (struct er_context){0};
    CHECK(er_clear(&o, drop, &c) == ER_OK && c.count == 4 &&
          o.epoch == UINT32_MAX);
    struct er_item full[12];
    for (unsigned i = 0; i < 12; ++i)
        full[i] = (struct er_item){i + 1, (int)i};
    CHECK(er_init(&o, full, 12) == ER_OK);
    before = o;
    CHECK(er_init(&o, full, 13) == ER_INVALID &&
          !memcmp(&o, &before, sizeof o));
    full[11].id = 1;
    CHECK(er_init(&o, full, 12) == ER_INVALID &&
          !memcmp(&o, &before, sizeof o));
    full[11].id = 12;
    c = (struct er_context){.mask = 4095};
    CHECK(er_remove_if(&o, predicate, &c, &second) == ER_OK && o.count == 12 &&
          o.keep == 0 && !c.count);
    CHECK(er_erase(&o, second, drop, &c) == ER_OK && c.count == 12 && !o.count);
    CHECK(er_init(&o, full, 12) == ER_OK);
    c = (struct er_context){0};
    CHECK(er_remove_if(&o, predicate, &c, &second) == ER_OK && o.keep == 12);
    CHECK(er_erase(&o, second, drop, &c) == ER_OK && !c.count && o.count == 12);
    CHECK(er_clear(&o, drop, &c) == ER_OK && c.count == 12);
    puts(
        "ER_PASS stable-prefix distinct-tail failed-predicate stale-token exact-rights");
    return 0;
}
