#include "receivers.h"
#include "receiver_macros.h"
#include "metaprog.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                 \
    do                                                           \
    {                                                            \
        if (!(x))                                                \
        {                                                        \
            fprintf(stderr, "MPO_CHECK %d: %s\n", __LINE__, #x); \
            return 1;                                            \
        }                                                        \
    } while (0)

static uint32_t *mutable_pointer(uint32_t *p, unsigned *calls)
{
    ++*calls;
    return p;
}

static volatile uint32_t *volatile_pointer(volatile uint32_t *p,
                                           unsigned *calls)
{
    ++*calls;
    return p;
}

struct mp_change
{
    volatile uint32_t *p;
    uint32_t next;
    unsigned calls;
};

static void change(void *context)
{
    struct mp_change *c = context;
    ++c->calls;
    *c->p = c->next;
}

static int dispatch(void)
{
    uint32_t a = 7;
    const uint32_t b = 9;
    volatile uint32_t c = 11;
    const volatile uint32_t d = 13;
    uint32_t *const pointer = &a;
    struct mpv_read out = {91, 92};
    unsigned calls = 0;
    CHECK(MPV_READ(mutable_pointer(&a, &calls), &out) == MPV_OK &&
          out.value == 7 && out.category == 1 && calls == 1);
    CHECK(MPV_READ(&b, &out) == MPV_OK && out.value == 9 && out.category == 2);
    CHECK(MPV_READ(volatile_pointer(&c, &calls), &out) == MPV_OK &&
          out.value == 11 && out.category == 3 && calls == 2);
    CHECK(MPV_READ(&d, &out) == MPV_OK && out.value == 13 && out.category == 4);
    CHECK(MPV_READ(pointer, &out) == MPV_OK && out.category == 1 && a == 7 &&
          b == 9 && c == 11 && d == 13);
    out = (struct mpv_read){91, 92};
    CHECK(MPV_READ((volatile uint32_t *)NULL, &out) == MPV_INVALID &&
          out.value == 91 && out.category == 92);
    CHECK(MPV_READ(&c, NULL) == MPV_INVALID && c == 11);
    puts("MPV_DISPATCH four-pointee-qualifiers unevaluated-control once");
    return 0;
}

static int effects(void)
{
    volatile uint32_t value = 17;
    const volatile uint32_t *readonly = &value;
    struct mp_change c = {&value, 23, 0};
    struct mpv_sample out = {91, 92};
    CHECK(MPV_OBSERVE(&value, change, &c, &out) == MPV_OK && out.before == 17 &&
          out.after == 23 && c.calls == 1);
    c.next = UINT32_MAX;
    CHECK(MPV_OBSERVE(readonly, change, &c, &out) == MPV_OK &&
          out.before == 23 && out.after == UINT32_MAX && c.calls == 2);
    out = (struct mpv_sample){91, 92};
    CHECK(MPV_OBSERVE(readonly, NULL, &c, &out) == MPV_INVALID &&
          out.before == 91 && out.after == 92 && c.calls == 2);
    CHECK(MPV_OBSERVE(readonly, change, &c, NULL) == MPV_INVALID &&
          c.calls == 2);
    CHECK(MPV_OBSERVE((const volatile uint32_t *)NULL, change, &c, &out) ==
              MPV_INVALID &&
          out.before == 91 && c.calls == 2);
    puts("MPV_EFFECTS read callback read explicit-borrow mutable-alias");
    return 0;
}

static int device(void)
{
    volatile uint32_t word = 0xa5;
    uint32_t plain = 19;
    struct mpv_rw rw = {&word, 15};
    struct mpv_w1_c w1c = {&word, 15};
    unsigned calls = 0;
    CHECK(mpv_update(rw, 5, 2) == MPV_OK && word == 0xa2);
    CHECK(mpv_update(rw, 16, 0) == MPV_RANGE && word == 0xa2);
    CHECK(mpv_ack(w1c, 4) == MPV_OK && word == 4);
    CHECK(mpv_ack(w1c, 16) == MPV_RANGE && word == 4);
    CHECK(MPV_WRITE(volatile_pointer(&word, &calls), 15, 16) == MPV_RANGE &&
          word == 4 && calls == 1);
    CHECK(MPV_WRITE(&word, 15, 7) == MPV_OK && word == 7);
    CHECK(MPV_WRITE(&plain, 0, 0) == MPV_OK && plain == 0);
    CHECK(MPV_WRITE(&plain, 0, 1) == MPV_RANGE && plain == 0);
    CHECK(mpv_ack((struct mpv_w1_c){NULL, 15}, 4) == MPV_INVALID &&
          mpv_update((struct mpv_rw){NULL, 15}, 1, 2) == MPV_INVALID);
    puts("MPV_DEVICE admitted-rw one-read-one-write w1c-command no-read");
    return 0;
}

static int baseline(void)
{
    uint32_t m = 7, out = 91;
    const uint32_t c = 9;
    unsigned calls = 92;
    CHECK(mp_generic_receivers(&m, &c, &calls, &out) && out == 316 &&
          calls == 2 && m == 7 && c == 9);
    m = UINT32_MAX;
    CHECK(mp_generic_receivers(&m, &c, &calls, &out) && out == 308 &&
          calls == 2 && m == UINT32_MAX);
    puts("MPV_BASE actual-const-policy two-evaluations modulo");
    return 0;
}

static int driver(void)
{
    unsigned op, nullp, nullo;
    uint32_t initial, mask, first, second;
    while (scanf("%u%" SCNu32 "%" SCNu32 "%" SCNu32 "%" SCNu32 "%u%u", &op,
                 &initial, &mask, &first, &second, &nullp, &nullo) == 7)
    {
        uint32_t plain = initial;
        volatile uint32_t word = initial;
        struct mpv_read r = {91, 92};
        struct mpv_sample s = {91, 92};
        struct mp_change c = {&word, first, 0};
        int status;
        uint32_t a = 91, b = 92;
        switch (op)
        {
            case 0:
                status = MPV_READ(nullp ? (uint32_t *)NULL : &plain,
                                  nullo ? NULL : &r);
                break;
            case 1:
                status = MPV_READ(nullp ? (const uint32_t *)NULL
                                        : (const uint32_t *)&plain,
                                  nullo ? NULL : &r);
                break;
            case 2:
                status = MPV_READ(nullp ? (volatile uint32_t *)NULL : &word,
                                  nullo ? NULL : &r);
                break;
            case 3:
                status = MPV_READ(nullp ? (const volatile uint32_t *)NULL
                                        : (const volatile uint32_t *)&word,
                                  nullo ? NULL : &r);
                break;
            case 4:
                status =
                    MPV_WRITE(nullp ? (uint32_t *)NULL : &plain, mask, first);
                break;
            case 5:
                status = MPV_WRITE(nullp ? (volatile uint32_t *)NULL : &word,
                                   mask, first);
                break;
            case 6:
                status = mpv_update((struct mpv_rw){nullp ? NULL : &word, mask},
                                    first, second);
                break;
            case 7:
                status = mpv_ack((struct mpv_w1_c){nullp ? NULL : &word, mask},
                                 first);
                break;
            case 8:
                status = MPV_OBSERVE(nullp ? (volatile uint32_t *)NULL : &word,
                                     change, &c, nullo ? NULL : &s);
                break;
            case 9:
                status = MPV_OBSERVE(nullp ? (const volatile uint32_t *)NULL
                                           : (const volatile uint32_t *)&word,
                                     change, &c, nullo ? NULL : &s);
                break;
            default:
                return 2;
        }
        if (op < 4)
        {
            a = r.value;
            b = r.category;
        }
        else if (op >= 8)
        {
            a = s.before;
            b = s.after;
        }
        printf("%d %" PRIu32 " %" PRIu32 " %" PRIu32 " %u\n", status,
               (op < 2 || op == 4) ? plain : word, a, b, c.calls);
    }
    return ferror(stdin) ? 2 : 0;
}

static int metrics(void)
{
    volatile uint32_t value = 7;
    struct mpv_read out = {0};
    uint64_t sum = 0;
    double generic[7], direct[7];
    unsigned sample, round;
    for (sample = 0; sample < 7; ++sample)
    {
        clock_t start = clock();
        CHECK(start != (clock_t)-1);
        sum = 0;
        for (round = 0; round < 10000; ++round)
        {
            CHECK(MPV_READ(&value, &out) == MPV_OK && out.category == 3);
            sum += out.value;
        }
        generic[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
        CHECK(sum == 70000);
        start = clock();
        CHECK(start != (clock_t)-1);
        sum = 0;
        for (round = 0; round < 10000; ++round)
        {
            CHECK(mpv_read_v(&value, &out) == MPV_OK && out.category == 3);
            sum += out.value;
        }
        direct[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
        CHECK(sum == 70000);
    }
    printf(
        "{\"iterations\":10000,\"checksum\":70000,\"read_size\":%zu,\"rw_size\":%zu,\"w1c_size\":%zu,\"generic_seconds\":[",
        sizeof(struct mpv_read), sizeof(struct mpv_rw),
        sizeof(struct mpv_w1_c));
    for (sample = 0; sample < 7; ++sample)
    {
        printf("%s%.9f", sample ? "," : "", generic[sample]);
    }
    printf("],\"direct_seconds\":[");
    for (sample = 0; sample < 7; ++sample)
    {
        printf("%s%.9f", sample ? "," : "", direct[sample]);
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
    if (!strcmp(argv[1], "receiver_dispatch"))
    {
        return dispatch();
    }
    if (!strcmp(argv[1], "receiver_effects"))
    {
        return effects();
    }
    if (!strcmp(argv[1], "receiver_device"))
    {
        return device();
    }
    if (!strcmp(argv[1], "receiver_base"))
    {
        return baseline();
    }
    if (!strcmp(argv[1], "driver"))
    {
        return driver();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        return metrics();
    }
    return 2;
}
