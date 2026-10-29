#include "serial_timer.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "ER_CHECK EAT line%d: %s\n", __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)

static int base(void)
{
    uint32_t out = 91;
    int order = 7;
    CHECK(er_add16(65535, 1, ER_CHECKED, &out) == ER_RANGE && out == 91);
    CHECK(er_add16(65535, 1, ER_WRAPPING, &out) == ER_OK && out == 0);
    CHECK(er_add16(65535, 1, ER_SATURATING, &out) == ER_OK && out == 65535);
    CHECK(er_add16(UINT32_MAX, 0, ER_CHECKED, &out) == ER_INVALID &&
          out == 65535);
    CHECK(er_serial16(32768, 0, &order) == ER_RANGE && order == 7);
    CHECK(er_serial16(0, 65535, &order) == ER_OK && order == 1);
    puts("EAT_BASE actual-add16-policy unchanged-overflow serial-half-range");
    return 0;
}

static int relation(void)
{
    for (unsigned bits = 2; bits <= 16; ++bits)
    {
        uint32_t m = UINT32_C(1) << bits;
        int order = 9;
        CHECK(eat_serial(bits, m / 2, 0, &order) == ER_RANGE && order == 9);
        CHECK(eat_serial(bits, 0, m - 1, &order) == ER_OK && order == 1);
        CHECK(eat_serial(bits, m - 1, 0, &order) == ER_OK && order == -1);
        CHECK(eat_serial(bits, m, 0, &order) == ER_INVALID && order == -1);
    }
    int a, b, c;
    CHECK(eat_serial(4, 0, 11, &a) == ER_OK && a == 1 &&
          eat_serial(4, 11, 6, &b) == ER_OK && b == 1 &&
          eat_serial(4, 6, 0, &c) == ER_OK && c == 1);
    puts(
        "EAT_RELATION widths-two-through-sixteen ambiguous-preserves three-cycle");
    return 0;
}

static int window(void)
{
    struct eat_window q;
    uint32_t id = 99;
    CHECK(eat_window_init(&q, 16, 65534) == ER_OK);
    CHECK(eat_window_arm(&q, 11, 2) == ER_OK &&
          eat_window_arm(&q, 22, 0) == ER_OK &&
          eat_window_arm(&q, 33, 2) == ER_OK);
    CHECK(eat_window_pop(&q, &id) == ER_OK && id == 22);
    CHECK(eat_window_pop(&q, &id) == ER_AGAIN && id == 22);
    CHECK(eat_window_advance(&q, 3) == ER_OK && q.now == 1);
    CHECK(eat_window_pop(&q, &id) == ER_OK && id == 11);
    CHECK(eat_window_pop(&q, &id) == ER_OK && id == 33 && q.count == 0);
    puts(
        "EAT_WINDOW actual-serial-import wrapped-deadline oldest-first stable-tie");
    return 0;
}

static int lifetime(void)
{
    struct eat_window q;
    CHECK(eat_window_init(&q, 4, 14) == ER_OK &&
          eat_window_arm(&q, 1, 3) == ER_OK &&
          eat_window_advance(&q, 7) == ER_OK);
    struct eat_window old = q;
    CHECK(eat_window_advance(&q, 1) == ER_RANGE &&
          memcmp(&q, &old, sizeof q) == 0);
    uint32_t id = 99;
    CHECK(eat_window_pop(&q, &id) == ER_OK && id == 1);
    CHECK(eat_window_advance(&q, 7) == ER_OK);
    int order;
    CHECK(
        eat_serial(4, 0, 1, &order) == ER_OK &&
        order ==
            -1); /* due at absolute1, now16 is old; due17 is new: identical low pair */
    puts(
        "EAT_LIFETIME accumulated-residence-refuses-before-half-window identical-low-stale-new");
    return 0;
}

static int epoch(void)
{
    struct eat_epoch q;
    uint32_t id = 99;
    CHECK(eat_epoch_init(&q, 4, 0) == ER_OK &&
          eat_epoch_arm(&q, 11, 1) == ER_OK &&
          eat_epoch_advance(&q, 16) == ER_OK &&
          eat_epoch_arm(&q, 22, 1) == ER_OK);
    CHECK(eat_epoch_pop(&q, &id) == ER_OK && id == 11);
    CHECK(eat_epoch_pop(&q, &id) == ER_AGAIN && id == 11);
    CHECK(eat_epoch_advance(&q, 1) == ER_OK &&
          eat_epoch_pop(&q, &id) == ER_OK && id == 22);
    CHECK(eat_epoch_init(&q, 16, UINT64_MAX - 1) == ER_OK &&
          eat_epoch_arm(&q, 9, 1) == ER_OK);
    struct eat_epoch old = q;
    CHECK(eat_epoch_advance(&q, 2) == ER_RANGE &&
          memcmp(&q, &old, sizeof q) == 0);
    CHECK(eat_epoch_arm(&q, 8, 2) == ER_RANGE &&
          memcmp(&q, &old, sizeof q) == 0);
    CHECK(eat_epoch_advance(&q, 1) == ER_OK &&
          eat_epoch_pop(&q, &id) == ER_OK && id == 9);
    CHECK(eat_epoch_arm(&q, 0, 0) == ER_OK && eat_epoch_pop(&q, &id) == ER_OK &&
          id == 0);
    puts(
        "EAT_EPOCH low-width-independent old-new-distinct full-width-exhaustion-drain");
    return 0;
}

static int admission(void)
{
    struct eat_window q;
    CHECK(eat_window_init(&q, 4, 0) == ER_OK);
    struct eat_window old = q;
    CHECK(eat_window_init(&q, 1, 0) == ER_INVALID &&
          memcmp(&q, &old, sizeof q) == 0);
    CHECK(eat_window_arm(&q, 1, 8) == ER_RANGE &&
          memcmp(&q, &old, sizeof q) == 0);
    CHECK(eat_window_advance(&q, 8) == ER_RANGE &&
          memcmp(&q, &old, sizeof q) == 0);
    for (unsigned i = 0; i < 4; ++i)
        CHECK(eat_window_arm(&q, i, 0) == ER_OK);
    old = q;
    CHECK(eat_window_arm(&q, 0, 0) == ER_INVALID &&
          memcmp(&q, &old, sizeof q) == 0);
    CHECK(eat_window_arm(&q, 5, 0) == ER_RANGE &&
          memcmp(&q, &old, sizeof q) == 0);
    CHECK(eat_window_pop(&q, NULL) == ER_INVALID &&
          memcmp(&q, &old, sizeof q) == 0);
    struct eat_epoch e;
    CHECK(eat_epoch_init(&e, 4, 0) == ER_OK);
    for (unsigned i = 0; i < 4; ++i)
        CHECK(eat_epoch_arm(&e, i, 0) == ER_OK);
    struct eat_epoch before = e;
    CHECK(eat_epoch_arm(&e, 0, 0) == ER_INVALID &&
          memcmp(&e, &before, sizeof e) == 0);
    CHECK(eat_epoch_arm(&e, 5, 0) == ER_RANGE &&
          memcmp(&e, &before, sizeof e) == 0);
    CHECK(eat_epoch_pop(NULL, NULL) == ER_INVALID);
    puts(
        "EAT_ADMISSION finite-capacity duplicate-before-full invalid-unchanged");
    return 0;
}

static int oracle(void)
{
    unsigned kind, bits;
    uint64_t start, a, b, elapsed;
    while (scanf("%u %u %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64, &kind,
                 &bits, &start, &a, &b, &elapsed) == 6)
    {
        if (kind == 0)
        {
            uint32_t out = 123456;
            er_code r =
                er_add16((uint32_t)a, (uint32_t)b, (er_policy)bits, &out);
            printf("%d %" PRIu32 "\n", r, out);
        }
        else if (kind == 1)
        {
            int out = 9;
            er_code r = eat_serial(bits, (uint32_t)a, (uint32_t)b, &out);
            printf("%d %d\n", r, out);
        }
        else
        {
            uint32_t x = 99, y = 98;
            er_code r0, r1, r2, r3, r4, r5;
            uint64_t now;
            size_t count;
            if (kind == 2)
            {
                struct eat_window q = {0};
                r0 = eat_window_init(&q, bits, (uint32_t)start);
                r1 = eat_window_arm(&q, 11, (uint32_t)a);
                r2 = eat_window_arm(&q, 22, (uint32_t)b);
                r3 = eat_window_advance(&q, (uint32_t)elapsed);
                now = q.now;
                r4 = eat_window_pop(&q, &x);
                r5 = eat_window_pop(&q, &y);
                count = q.count;
            }
            else
            {
                struct eat_epoch q = {0};
                r0 = eat_epoch_init(&q, bits, start);
                r1 = eat_epoch_arm(&q, 11, a);
                r2 = eat_epoch_arm(&q, 22, b);
                r3 = eat_epoch_advance(&q, elapsed);
                now = q.now;
                r4 = eat_epoch_pop(&q, &x);
                r5 = eat_epoch_pop(&q, &y);
                count = q.count;
            }
            printf("%d %d %d %d %" PRIu64 " %d %" PRIu32 " %d %" PRIu32
                   " %zu\n",
                   r0, r1, r2, r3, now, r4, x, r5, y, count);
        }
    }
    return ferror(stdin) ? 2 : 0;
}

static int metrics(void)
{
    double samples[7];
    unsigned long sum = 0;
    for (unsigned j = 0; j < 7; ++j)
    {
        clock_t start = clock();
        for (unsigned i = 0; i < 10000; ++i)
        {
            struct eat_epoch q;
            uint32_t id = 0;
            CHECK(eat_epoch_init(&q, 4, 0) == ER_OK &&
                  eat_epoch_arm(&q, 11, 1) == ER_OK &&
                  eat_epoch_advance(&q, 16) == ER_OK &&
                  eat_epoch_pop(&q, &id) == ER_OK);
            sum += id;
        }
        samples[j] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf(
        "{\"iterations\":10000,\"checksum\":%lu,\"allocation_calls\":0,\"window_size\":%zu,\"epoch_size\":%zu,\"seconds\":[",
        sum, sizeof(struct eat_window), sizeof(struct eat_epoch));
    for (unsigned i = 0; i < 7; ++i)
        printf("%s%.9f", i ? "," : "", samples[i]);
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "timer_base"))
        return base();
    if (!strcmp(argv[1], "timer_relation"))
        return relation();
    if (!strcmp(argv[1], "timer_window"))
        return window();
    if (!strcmp(argv[1], "timer_lifetime"))
        return lifetime();
    if (!strcmp(argv[1], "timer_epoch"))
        return epoch();
    if (!strcmp(argv[1], "timer_admission"))
        return admission();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    return 2;
}
