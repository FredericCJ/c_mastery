#include "coroutines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CR_DEPTH:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int iterator(void)
{
    struct co_flow f = {{65530, 65536, 1, 0}, 9, 0};
    uint32_t out = 91;
    for (uint32_t x = 65530; x < 65536; x += 2)
        C(co_flow_next(&f, &out) == 0 && out == (uint32_t)((uint64_t)x * x));
    C(co_flow_next(&f, &out) == CO_DONE && out == 4294705156u);
    struct co_range r = {65535, 65536, UINT32_MAX, 0};
    C(co_next(&r, &out) == 0 && out == 65535 && r.current == 65536);
    C(co_next(&r, &out) == CO_DONE && out == 65535);
    return 0;
}

struct co_a
{
    unsigned live, fail;
};

static void *allocate(void *v, size_t n)
{
    struct co_a *a = v;
    if (a->fail)
        return NULL;
    void *p = malloc(n);
    if (p)
        a->live++;
    return p;
}

static void release(void *v, void *p)
{
    struct co_a *a = v;
    a->live--;
    free(p);
}

static int generators(void)
{
    struct co_a a = {0};
    struct co_allocator p = {&a, allocate, release};
    struct co_generator *g = NULL, *h = NULL;
    uint32_t out = 91;
    for (unsigned n = 0; n <= 47; n++)
    {
        C(co_generator_create(p, n, &g) == 0 &&
          co_generator_create(p, 47, &h) == 0);
        uint64_t x = 0, y = 1;
        for (unsigned i = 0; i < n; i++)
        {
            C(co_generator_next(g, &out) == 0 && out == x);
            uint32_t other = 91;
            C(co_generator_next(h, &other) == 0 && other == x);
            uint64_t next = x + y;
            x = y;
            y = next;
        }
        out = 91;
        C(co_generator_next(g, &out) == CO_DONE && out == 91);
        C(co_generator_destroy(&g) == 0 && co_generator_destroy(&h) == 0 &&
          !a.live);
        C(co_generator_destroy(&g) == 0);
    }
    a.fail = 1;
    C(co_generator_create(p, 1, &g) == CO_RANGE && !g && !a.live);
    return 0;
}

static int factorial(void)
{
    unsigned comparisons = 0;
    uint32_t mathematical = 1;
    for (unsigned n = 0; n <= 12; n++)
    {
        if (n)
            mathematical *= n;
        struct co_trampoline origin;
        C(co_trampoline_init(&origin, n) == 0);
        for (unsigned cut = 0; cut <= 2 * n + 2; cut++)
        {
            struct co_trampoline state = origin;
            uint32_t q = 91;
            int reached = co_trampoline_step(&state, cut, &q);
            C(state.depth <= n);
            if (reached == CO_DONE)
                C(q == mathematical && state.depth == 0);
            else
                C(q == 91);
            for (unsigned a = 0; a <= 28; a++)
                for (unsigned b = 0; b <= 28; b++)
                {
                    struct co_trampoline one = state, two = state;
                    uint32_t x = 91, y = 91;
                    int r = co_trampoline_step(&one, a + b, &x);
                    (void)co_trampoline_step(&two, a, &y);
                    int s = co_trampoline_step(&two, b, &y);
                    C(r == s && memcmp(&one, &two, sizeof one) == 0 && x == y);
                    comparisons++;
                }
            struct co_trampoline cancelled = state;
            C(co_trampoline_cancel(&cancelled) == 0 && cancelled.depth == 0);
            q = 91;
            C(co_trampoline_step(&cancelled, 100, &q) == CO_STATE && q == 91);
        }
    }
    printf(
        "CR_PARTITIONS:%u pairwise budget-composition cases on every reachable state; induction covers arbitrary finite partitions\n",
        comparisons);
    return 0;
}

static int parsers(void)
{
    struct co_parser p[] = {{CO_CHOICE, 1, 4, 0},
                            {CO_SEQUENCE, 2, 3, 0},
                            {CO_LITERAL, 0, 0, 'a'},
                            {CO_LITERAL, 0, 0, 'b'},
                            {CO_LITERAL, 0, 0, 'a'}};
    unsigned cases = 0;
    for (size_t n = 0; n <= 7; n++)
    {
        unsigned limit = 1;
        for (size_t i = 0; i < n; i++)
            limit *= 3;
        for (unsigned code = 0; code < limit; code++)
        {
            unsigned value = code;
            unsigned char input[7];
            for (size_t i = 0; i < n; i++)
            {
                input[i] = (const unsigned char[]){'a', 'b', 'x'}[value % 3];
                value /= 3;
            }
            size_t end = 91;
            unsigned steps = 99;
            int r = co_parse(p, 5, input, n, 0, 50, &end, &steps);
            size_t expected =
                n && input[0] == 'a' ? (n > 1 && input[1] == 'b' ? 2 : 1) : 91;
            C(r == (expected == 91 ? CO_NO_MATCH : CO_OK) && end == expected &&
              steps <= 5);
            cases++;
        }
    }
    size_t end = 91;
    unsigned steps = 77;
    struct co_parser bad[] = {{CO_SEQUENCE, 0, 1, 0}, {CO_EMPTY, 0, 0, 0}};
    C(co_parse(bad, 2, NULL, 0, 0, 20, &end, &steps) == CO_INVALID &&
      end == 91 && steps == 77);
    struct co_parser stalled[] = {{CO_MANY, 1, 0, 0}, {CO_EMPTY, 0, 0, 0}};
    C(co_parse(stalled, 2, NULL, 0, 0, 20, &end, &steps) == CO_STATE &&
      end == 91);
    C(co_parse(p, 5, NULL, 0, 0, 0, &end, &steps) == CO_RANGE && steps == 0 &&
      end == 91);
    printf("CR_PARSER:%u independent prefix-grammar cases\n", cases);
    return 0;
}

struct co_port
{
    struct co_read_result result[8];
    unsigned length, at, reads, arms, disarms, fail_arm;
    uint32_t generation;
};

static struct co_read_result read_port(void *v, unsigned char *out,
                                       size_t capacity)
{
    struct co_port *p = v;
    p->reads++;
    struct co_read_result r =
        p->result[p->at < p->length ? p->at++ : p->length - 1];
    for (size_t i = 0; i < r.count && i < capacity; i++)
        out[i] = (unsigned char)(20 + i);
    return r;
}

static int arm_port(void *v, uint32_t generation)
{
    struct co_port *p = v;
    p->arms++;
    p->generation = generation;
    return p->fail_arm ? 1 : 0;
}

static void disarm_port(void *v, uint32_t generation)
{
    struct co_port *p = v;
    if (generation == p->generation)
        p->disarms++;
}

static int readiness(void)
{
    for (unsigned terminal = CO_READ_EOF; terminal <= CO_READ_ERROR; terminal++)
    {
        struct co_port p = {.result = {{CO_READ_DATA, 1}, {terminal, 1}},
                            .length = 2};
        struct co_async_port port = {&p, read_port, arm_port, disarm_port};
        struct co_async f;
        unsigned char out[8] = {91};
        size_t n = 92;
        C(co_async_init(&f, 4, 1) == 0);
        C(co_async_poll(&f, port, out, &n) ==
          (terminal == CO_READ_EOF ? CO_NO_MATCH : CO_IO));
        C(f.length == 2 && p.reads == 2 && p.arms == 1 && p.disarms == 1 &&
          out[0] == 91 && n == 92);
        C(co_async_notify(&f, 1) == CO_STATE &&
          co_async_poll(&f, port, out, &n) == CO_STATE && p.reads == 2);
    }
    struct co_port p = {.result = {{CO_READ_AGAIN, 0}}, .length = 1};
    struct co_async_port port = {&p, read_port, arm_port, disarm_port};
    struct co_async f;
    unsigned char out[8] = {91};
    size_t n = 92;
    C(co_async_init(&f, 2, 9) == 0);
    C(co_async_poll(&f, port, out, &n) == CO_PENDING && p.reads == 2);
    for (unsigned i = 0; i < 10; i++)
    {
        C(co_async_notify(&f, 8) == CO_STATE && co_async_notify(&f, 9) == 0);
        C(co_async_poll(&f, port, out, &n) == CO_PENDING);
    }
    C(p.reads == 12 && p.arms == 1 && !p.disarms && out[0] == 91 && n == 92);
    C(co_async_cancel(&f, port) == 0 && p.disarms == 1);
    p = (struct co_port){
        .result = {{CO_READ_AGAIN, 0}}, .length = 1, .fail_arm = 1};
    C(co_async_init(&f, 2, 10) == 0);
    C(co_async_poll(&f, port, out, &n) == CO_IO && p.reads == 1 &&
      p.arms == 1 && !p.disarms && !f.registered && out[0] == 91 && n == 92);
    return 0;
}

static int effects(void)
{
    for (unsigned abort_at = 1; abort_at <= 2; abort_at++)
    {
        struct co_effect f;
        struct co_continuation a, b, old;
        uint32_t out = 91;
        C(co_effect_start(&f, &a) == CO_REQUEST);
        old = a;
        if (abort_at == 2)
        {
            C(co_effect_resume(&a, UINT32_MAX, &b, &out) == CO_REQUEST);
            C(co_effect_resume(&old, 0, &a, &out) == CO_STATE);
            old = b;
            C(co_effect_resume(&b, 1, &a, &out) == CO_RANGE && out == 91 &&
              b.owner == &f);
            a = b;
        }
        C(co_effect_abort(&a) == 0 &&
          co_effect_resume(&old, 0, &b, &out) == CO_STATE && out == 91);
    }
    struct co_effect f;
    struct co_continuation a, b, first, second;
    uint32_t out = 91;
    C(co_effect_start(&f, &a) == CO_REQUEST);
    first = a;
    C(co_effect_resume(&a, UINT32_MAX, &b, &out) == CO_REQUEST);
    second = b;
    C(co_effect_resume(&b, 1, &a, &out) == CO_RANGE);
    C(co_effect_resume(&b, 0, &a, &out) == CO_DONE && out == UINT32_MAX);
    C(co_effect_resume(&first, 0, &a, &out) == CO_STATE &&
      co_effect_resume(&second, 0, &a, &out) == CO_STATE);
    /* All previous tokens are discarded before fresh initialization: the
     * original contract explicitly forbids reinitialization ABA. */
    first = (struct co_continuation){0};
    second = (struct co_continuation){0};
    C(co_effect_start(&f, &a) == CO_REQUEST && co_effect_abort(&a) == 0);
    return 0;
}

int main(void)
{
    C(iterator() == 0 && generators() == 0 && factorial() == 0 &&
      parsers() == 0 && readiness() == 0 && effects() == 0);
    puts("CR_DEPTH:all retained mechanism boundary probes passed");
    return 0;
}
