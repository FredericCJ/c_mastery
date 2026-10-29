#include "foreign_backend.h"
#include "object_runtime_ext.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                               \
    do                                                         \
    {                                                          \
        if (!(c))                                              \
        {                                                      \
            fprintf(stderr, "OR_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int eq(struct orx_handle a, struct orx_handle b)
{
    return a.slot == b.slot && a.generation == b.generation;
}

static int values(void)
{
    struct orx_value value = {0}, before;
    unsigned char wire[6] = {2, 0x7f, 0x81, 0, 1, 0}, round[6];
    uint64_t bits;
    double d;
    unsigned i;
    static const uint64_t bridge[] = {UINT64_C(0),
                                      UINT64_C(0x8000000000000000),
                                      UINT64_C(1),
                                      UINT64_C(0x000fffffffffffff),
                                      UINT64_C(0x0010000000000000),
                                      UINT64_C(0x7fefffffffffffff),
                                      UINT64_C(0x7ff0000000000000),
                                      UINT64_C(0xfff0000000000000),
                                      UINT64_C(0xfff0000000000001)};
    CHECK(orx_value_decode(wire, &value) == OR_OK && value.tag == 2 &&
          value.as.binary32 == UINT32_C(0x7fc00000));
    memcpy(&before, &value, sizeof value);
    wire[5] = 1;
    CHECK(orx_value_decode(wire, &value) == OR_INVALID &&
          !memcmp(&before, &value, sizeof value));
    value.tag = 1;
    value.as.integer = INT32_MIN;
    CHECK(orx_value_encode(value, round) == OR_OK && round[1] == 128);
    CHECK(orx_value_decode(round, &value) == OR_OK &&
          value.as.integer == INT32_MIN);
    CHECK(orx_double_bits(-0.0, &bits) == OR_OK &&
          bits == UINT64_C(0x8000000000000000));
    CHECK(orx_bits_double(UINT64_C(0x3ff4000000000000), &d) == OR_OK &&
          d == 1.25);
    CHECK(orx_bits_double(UINT64_C(0x7ff9000000000042), &d) == OR_OK);
    CHECK(orx_double_bits(d, &bits) == OR_OK &&
          bits == UINT64_C(0x7ff8000000000000));
    for (i = 0; i < sizeof bridge / sizeof bridge[0]; i++)
    {
        CHECK(orx_bits_double(bridge[i], &d) == OR_OK);
        CHECK(orx_double_bits(d, &bits) == OR_OK &&
              bits == or_number_bits(bridge[i]));
    }
    for (i = 0; i < 256; i++)
    {
        unsigned char invalid[6] = {(unsigned char)i, 0, 0, 0, 0, 0};
        if (i == 1 || i == 2)
        {
            continue;
        }
        memcpy(&before, &value, sizeof value);
        CHECK(orx_value_decode(invalid, &value) == OR_INVALID &&
              !memcmp(&before, &value, sizeof value));
    }
    return 0;
}

static int authority(void)
{
    struct orx_heap h, other;
    struct orx_handle object, other_object, out = {99, 99};
    struct orx_grant grant;
    struct orx_value value = {0};
    unsigned char round[6];
    orx_heap_init(&h);
    CHECK(orx_new(&h, (const unsigned char *)"local", 5, &object) == OR_OK);
    value.tag = 3;
    value.as.handle = object;
    CHECK(orx_value_encode(value, round) == OR_OK);
    CHECK(orx_value_decode(round, &value) == OR_OK);
    CHECK(orx_import(&h, value, NULL, &out) == OR_INVALID && out.slot == 99 &&
          h.entry[object.slot].roots == 1);
    orx_heap_init(&other);
    CHECK(orx_new(&other, (const unsigned char *)"other", 5, &other_object) ==
          OR_OK);
    grant = (struct orx_grant){&other, other_object};
    CHECK(orx_import(&h, value, &grant, &out) == OR_INVALID && out.slot == 99 &&
          h.entry[object.slot].roots == 1);
    grant = (struct orx_grant){&h, object};
    CHECK(orx_import(&h, value, &grant, &out) == OR_OK && eq(out, object) &&
          h.entry[object.slot].roots == 2);
    return 0;
}

static int pool(void)
{
    struct orx_heap h, before;
    struct orx_handle a, b, c, old;
    struct orx_ticket ticket;
    const unsigned char *raw;
    size_t n;
    unsigned moved, dead;
    unsigned char out[15];
    orx_heap_init(&h);
    CHECK(orx_new(&h, (const unsigned char *)"a", 1, &a) == OR_OK);
    CHECK(orx_new(&h, (const unsigned char *)"b", 1, &b) == OR_OK);
    CHECK(orx_foreign_begin(&h, b, 1, &ticket, &raw, &n) == OR_OK && n == 1 &&
          raw[0] == 'b');
    CHECK(orx_release(&h, a) == OR_OK);
    CHECK(orx_collect(&h, &dead) == OR_OK && dead == 1);
    CHECK(orx_pack(&h, &moved) == OR_OK && moved == 0 &&
          raw == h.cell[1].bytes && raw[0] == 'b');
    CHECK(orx_foreign_finish(ticket, 1) == OR_OK);
    CHECK(orx_pack(&h, &moved) == OR_OK && moved == 1);
    CHECK(orx_read(&h, b, out, &n) == OR_OK && n == 1 && out[0] == 'b');
    old = a;
    CHECK(orx_new(&h, NULL, 0, &a) == OR_OK && a.slot == old.slot &&
          a.generation == 2);
    memcpy(&before, &h, sizeof h);
    CHECK(orx_retain(&h, old) == OR_INVALID && !memcmp(&h, &before, sizeof h));
    CHECK(orx_new(&h, (const unsigned char *)"c", 1, &c) == OR_OK);
    CHECK(orx_edge(&h, a, 0, c) == OR_OK);
    CHECK(orx_edge(&h, c, 0, a) == OR_OK);
    CHECK(orx_release(&h, c) == OR_OK);
    CHECK(orx_collect(&h, &dead) == OR_OK && dead == 0);
    CHECK(orx_release(&h, a) == OR_OK);
    CHECK(orx_collect(&h, &dead) == OR_OK && dead == 2);
    CHECK(orx_new(&h, NULL, 0, &a) == OR_OK && a.generation == 3);
    CHECK(orx_release(&h, a) == OR_OK);
    CHECK(orx_collect(&h, &dead) == OR_OK && dead == 1 &&
          h.entry[a.slot].generation == 4);
    CHECK(orx_new(&h, NULL, 0, &c) == OR_OK && c.slot != a.slot);
    h.entry[c.slot].roots = UINT8_MAX;
    memcpy(&before, &h, sizeof h);
    CHECK(orx_retain(&h, c) == OR_FULL && !memcmp(&before, &h, sizeof h));
    h.epoch = UINT8_MAX;
    memcpy(&before, &h, sizeof h);
    dead = 91;
    CHECK(orx_collect(&h, &dead) == OR_FULL && dead == 91 &&
          !memcmp(&h, &before, sizeof h));
    return 0;
}

static int intern(void)
{
    struct orx_heap h, before;
    struct orx_intern t, oldtable;
    struct ow_allocator a = {0};
    struct orx_handle first, second, third, out = {99, 99};
    struct orx_ticket ticket;
    const unsigned char *pinned;
    unsigned dead, removed, moved;
    unsigned char bytes[15];
    size_t n;
    unsigned i;
    const unsigned char x[2] = {0, 97}, y[2] = {97, 0};
    orx_heap_init(&h);
    CHECK(orx_intern_init(&t, &h, ow_allocator(&a)) == OR_OK);
    CHECK(orx_intern_get(&t, x, 2, &first) == OR_OK && a.live == 1);
    CHECK(orx_intern_get(&t, y, 2, &second) == OR_OK && !eq(first, second));
    a.fail_at = a.attempts + 1;
    memcpy(&before, &h, sizeof h);
    oldtable = t;
    CHECK(orx_intern_get(&t, (const unsigned char *)"third", 5, &out) ==
              OR_FULL &&
          out.slot == 99 && !memcmp(&h, &before, sizeof h) &&
          t.bucket == oldtable.bucket && t.used == 2 && a.live == 1);
    CHECK(orx_intern_get(&t, x, 2, &out) == OR_OK && eq(out, first));
    CHECK(orx_release(&h, first) == OR_OK);
    a.fail_at = 0;
    CHECK(orx_intern_get(&t, (const unsigned char *)"third", 5, &third) ==
              OR_OK &&
          t.capacity == 8 && a.live == 1);
    CHECK(orx_release(&h, second) == OR_OK);
    CHECK(orx_collect(&h, &dead) == OR_OK && dead == 1);
    CHECK(orx_intern_get(&t, x, 2, &out) == OR_INVALID);
    CHECK(orx_intern_sweep(&t, &removed) == OR_OK && removed == 1);
    CHECK(orx_foreign_begin(&h, first, 1, &ticket, &pinned, &n) == OR_OK &&
          n == 2 && pinned[0] == 0 && pinned[1] == 97);
    CHECK(orx_pack(&h, &moved) == OR_OK && moved == 1);
    CHECK(pinned[0] == 0 && pinned[1] == 97);
    CHECK(orx_read(&h, third, bytes, &n) == OR_OK && n == 5 && bytes[0] == 't');
    a.fail_at = a.attempts + 1;
    memcpy(&before, &h, sizeof h);
    oldtable = t;
    CHECK(orx_intern_reserve(&t, 16) == OR_FULL &&
          !memcmp(&h, &before, sizeof h) && t.bucket == oldtable.bucket &&
          t.capacity == 8 && t.used == 2 && a.live == 1);
    CHECK(orx_intern_get(&t, (const unsigned char *)"third", 5, &out) ==
              OR_OK &&
          eq(out, third));
    CHECK(orx_release(&h, out) == OR_OK);
    CHECK(orx_foreign_cancel(ticket, 1) == OR_OK);
    /* This local observer has finished reading; no backend was started here.
       The separate foreign test establishes actual pthread join quiescence. */
    CHECK(orx_foreign_finish(ticket, 1) == OR_OK);
    a.fail_at = 0;
    CHECK(orx_intern_get(&t, x, 2, &out) == OR_OK && eq(out, first));
    CHECK(orx_release(&h, out) == OR_OK);
    CHECK(orx_intern_get(&t, y, 2, &out) == OR_OK && out.slot == second.slot &&
          out.generation != second.generation);
    CHECK(orx_intern_close(&t) == OR_OK && a.live == 0 && !a.invalid_releases);
    CHECK(orx_release(&h, first) == OR_OK && orx_release(&h, third) == OR_OK &&
          orx_release(&h, out) == OR_OK);
    CHECK(orx_collect(&h, &dead) == OR_OK && dead == 3);
    /* A private rehash allocation must unwind when the independently full pool
     * rejects. */
    orx_heap_init(&h);
    a = (struct ow_allocator){0};
    CHECK(orx_intern_init(&t, &h, ow_allocator(&a)) == OR_OK);
    for (i = 0; i < 8; i++)
    {
        unsigned char b = (unsigned char)i;
        CHECK(orx_new(&h, &b, 1, &out) == OR_OK);
    }
    out = (struct orx_handle){99, 99};
    CHECK(orx_intern_get(&t, NULL, 0, &out) == OR_FULL && a.live == 0 &&
          out.slot == 99 && t.bucket == NULL && t.used == 0);
    CHECK(orx_intern_close(&t) == OR_OK);
    for (i = 0; i < 8; i++)
    {
        CHECK(orx_release(&h, (struct orx_handle){(uint8_t)i, 1}) == OR_OK);
    }
    CHECK(orx_collect(&h, &dead) == OR_OK && dead == 8);
    return 0;
}

static int foreign(void)
{
    struct orx_heap h;
    struct orx_handle object, filler, other;
    struct orx_ticket ticket, stale, next;
    const unsigned char *raw;
    size_t length;
    unsigned n;
    struct orx_backend backend;
    pthread_t thread;
    void *result;
    /* Mutation controls never strand a real thread on an intended assertion
     * failure. */
    orx_heap_init(&h);
    CHECK(orx_new(&h, (const unsigned char *)"AB", 2, &object) == OR_OK);
    CHECK(orx_foreign_begin(&h, object, 7, &ticket, &raw, &length) == OR_OK);
    CHECK(orx_release(&h, object) == OR_OK);
    CHECK(orx_foreign_cancel(ticket, 7) == OR_OK);
    CHECK(orx_collect(&h, &n) == OR_OK && n == 0 &&
          h.entry[object.slot].pins == 1);
    stale = ticket;
    CHECK(orx_foreign_finish(ticket, 7) == OR_OK);
    CHECK(orx_foreign_begin(&h, object, 7, &next, &raw, &length) == OR_OK);
    CHECK(orx_foreign_finish(stale, 7) == OR_INVALID &&
          h.entry[object.slot].pins == 1);
    CHECK(orx_foreign_finish(next, 7) == OR_OK);
    orx_heap_init(&h);
    CHECK(orx_new(&h, (const unsigned char *)"x", 1, &filler) == OR_OK);
    CHECK(orx_new(&h, (const unsigned char *)"AB", 2, &object) == OR_OK);
    CHECK(orx_new(&h, (const unsigned char *)"z", 1, &other) == OR_OK);
    CHECK(orx_foreign_begin(&h, object, 7, &ticket, &raw, &length) == OR_OK);
    backend = (struct orx_backend){.bytes = raw, .length = length};
    CHECK(pthread_mutex_init(&backend.lock, NULL) == 0);
    CHECK(pthread_cond_init(&backend.changed, NULL) == 0);
    CHECK(pthread_create(&thread, NULL, orx_backend_worker, &backend) == 0);
    CHECK(pthread_mutex_lock(&backend.lock) == 0);
    while (!backend.ready)
    {
        CHECK(pthread_cond_wait(&backend.changed, &backend.lock) == 0);
    }
    CHECK(pthread_mutex_unlock(&backend.lock) == 0);
    /* Worker has borrowed bytes but cannot return until released below. */
    CHECK(orx_release(&h, filler) == OR_OK);
    CHECK(orx_release(&h, object) == OR_OK);
    CHECK(orx_collect(&h, &n) == OR_OK && n == 1);
    CHECK(orx_foreign_cancel(ticket, 7) == OR_OK);
    CHECK(orx_pack(&h, &n) == OR_OK && n == 1);
    CHECK(orx_collect(&h, &n) == OR_OK && n == 0);
    CHECK(pthread_mutex_lock(&backend.lock) == 0);
    backend.go = 1;
    CHECK(pthread_cond_signal(&backend.changed) == 0);
    CHECK(pthread_mutex_unlock(&backend.lock) == 0);
    CHECK(pthread_join(thread, &result) == 0 && result == NULL &&
          backend.sum == 131);
    CHECK(orx_foreign_finish(ticket, 7) == OR_OK);
    CHECK(orx_collect(&h, &n) == OR_OK && n == 1);
    CHECK(pthread_cond_destroy(&backend.changed) == 0);
    CHECK(pthread_mutex_destroy(&backend.lock) == 0);
    return 0;
}

static int oracle(void)
{
    static const unsigned char text[5][2] = {
        {0, 0}, {97, 0}, {98, 0}, {0, 97}, {97, 0}};
    static const unsigned length[5] = {0, 1, 1, 2, 2};
    unsigned sequence, fail, k, mask, i;
    for (sequence = 0; sequence < 625; sequence++)
    {
        for (fail = 0; fail < 4; fail++)
        {
            struct orx_heap h;
            struct orx_intern t;
            struct ow_allocator allocator = {0};
            unsigned word = sequence;
            struct orx_handle out;
            orx_heap_init(&h);
            CHECK(orx_intern_init(&t, &h, ow_allocator(&allocator)) == OR_OK);
            allocator.fail_at = fail;
            for (k = 0; k < 4; k++)
            {
                unsigned choice = word % 5;
                enum or_status status;
                word /= 5;
                out = (struct orx_handle){99, 99};
                status = orx_intern_get(&t, text[choice], length[choice], &out);
                printf("I %u %u %u %d %u %u %u %zu %zu\n", sequence, fail, k,
                       status, out.slot, out.generation, t.capacity,
                       allocator.attempts, allocator.live);
            }
            CHECK(orx_intern_close(&t) == OR_OK && allocator.live == 0);
        }
    }
    for (mask = 0; mask < 256; mask++)
    {
        struct orx_heap h;
        struct orx_handle handles[8];
        unsigned dead, moved;
        orx_heap_init(&h);
        for (i = 0; i < 8; i++)
        {
            unsigned char b = (unsigned char)(i + 11);
            CHECK(orx_new(&h, &b, 1, &handles[i]) == OR_OK);
            if (!(mask & (1u << i)))
            {
                CHECK(orx_release(&h, handles[i]) == OR_OK);
            }
        }
        CHECK(orx_collect(&h, &dead) == OR_OK);
        CHECK(orx_pack(&h, &moved) == OR_OK);
        printf("P %u %u %u", mask, dead, moved);
        for (i = 0; i < 8; i++)
        {
            printf(" %u %u %u", h.entry[i].live, h.entry[i].generation,
                   h.entry[i].live ? h.cell[h.entry[i].position].bytes[0] : 0);
        }
        puts("");
    }
    for (i = 0; i < 2; i++)
    {
        for (k = 0; k < 256; k++)
        {
            uint32_t b =
                (i ? UINT32_C(0x80000000) : 0) | UINT32_C(0x7f800000) | k;
            printf("V %u %u\n", b, orx_binary32(b));
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "values"))
    {
        return values();
    }
    if (!strcmp(argv[1], "authority"))
    {
        return authority();
    }
    if (!strcmp(argv[1], "pool"))
    {
        return pool();
    }
    if (!strcmp(argv[1], "intern"))
    {
        return intern();
    }
    if (!strcmp(argv[1], "foreign"))
    {
        return foreign();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    return 2;
}
