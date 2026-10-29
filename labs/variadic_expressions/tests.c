#include "format.h"
#include "expressions.h"
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "TF_FAIL %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                      \
        }                                                                  \
    } while (0)

/* Test-owned storage: no heap leak is needed to detect omitted release. */
struct tf_memory
{
    char stage[256];
    unsigned acquires, releases, live, fail;
    size_t bytes;
};

static void *allocate(void *v, size_t n)
{
    struct tf_memory *m = v;
    ++m->acquires;
    m->bytes = n;
    if (m->fail || n > sizeof m->stage || m->live)
        return NULL;
    m->live = 1;
    return m->stage;
}

static char *acquire(void *v, size_t n)
{
    return allocate(v, n);
}

static void release(void *v, char *p)
{
    struct tf_memory *m = v;
    if (p != m->stage || !m->live)
        abort();
    ++m->releases;
    m->live = 0;
}

struct tf_conversion
{
    unsigned calls, fail_at, change_at;
};

static int integer(void *v, char *out, size_t n, int value)
{
    struct tf_conversion *s = v;
    ++s->calls;
    if (s->calls == s->fail_at)
        return -1;
    int result = tf_default_integer(NULL, out, n, value);
    return s->calls == s->change_at && result >= 0 ? result + 1 : result;
}

static int real(void *v, char *out, size_t n, double value)
{
    struct tf_conversion *s = v;
    ++s->calls;
    if (s->calls == s->fail_at)
        return -1;
    int result = tf_default_real(NULL, out, n, value);
    return s->calls == s->change_at && result >= 0 ? result + 1 : result;
}

static int unchanged(const char *p, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        if (p[i] != '@')
            return 0;
    return 1;
}

static int formatting(void)
{
    for (size_t capacity = 0; capacity <= 14; ++capacity)
    {
        struct tf_memory m = {0};
        struct tf_conversion s = {0};
        struct tf_allocator a = {&m, acquire, release};
        struct tf_scalars scalar = {&s, integer, real};
        char out[64];
        memset(out, '@', sizeof out);
        size_t required = 99;
        enum tf_status status =
            tf_format(capacity ? out : NULL, capacity, &required, a, scalar, 2u,
                      'I', 7, 'D', 1.25);
        CHECK(status == (capacity <= 11 ? TF_FULL : TF_OK));
        CHECK(required == 11);
        if (capacity <= 11)
        {
            CHECK(unchanged(out, sizeof out));
            CHECK(m.acquires == 0 && s.calls == 2);
        }
        else
        {
            CHECK(strcmp(out, "I=7;D=1.25;") == 0 && out[12] == '@');
            CHECK(m.bytes == 12 && s.calls == 4);
            CHECK(m.releases == 1 && m.live == 0);
        }
    }
    struct tf_memory m = {0};
    struct tf_conversion s = {0};
    struct tf_allocator a = {&m, acquire, release};
    struct tf_scalars scalar = {&s, integer, real};
    char out[128];
    size_t required = 99;
    memset(out, '@', sizeof out);
    signed char ch = -2;
    short small = 7;
    float f = -0.5f;
    CHECK(tf_format(out, sizeof out, &required, a, scalar, 2u, 'I', ch, 'D',
                    f) == TF_OK);
    CHECK(strcmp(out, "I=-2;D=-0.50;") == 0 && required == 13);
    CHECK(tf_format(out, sizeof out, &required, a, scalar, 4u, 'I', small, 'D',
                    1.25, 'I', (char)3, 'D', (float)2.5) == TF_OK);
    CHECK(strcmp(out, "I=7;D=1.25;I=3;D=2.50;") == 0 && required == 22);
    puts("TF_PROMOTION_PASS char short float double exact_protocol");
    for (unsigned fail = 1; fail <= 4; ++fail)
    {
        m = (struct tf_memory){0};
        s = (struct tf_conversion){.fail_at = fail};
        required = 99;
        memset(out, '@', sizeof out);
        CHECK(tf_format(out, sizeof out, &required, a, scalar, 2u, 'I', 7, 'D',
                        1.25) == TF_FORMAT);
        CHECK(required == 99 && unchanged(out, sizeof out));
        CHECK(s.calls == fail);
        CHECK(m.acquires == (fail > 2) && m.releases == (fail > 2) && !m.live);
    }
    for (unsigned changed = 3; changed <= 4; ++changed)
    {
        m = (struct tf_memory){0};
        s = (struct tf_conversion){.change_at = changed};
        required = 99;
        memset(out, '@', sizeof out);
        CHECK(tf_format(out, sizeof out, &required, a, scalar, 2u, 'I', 7, 'D',
                        1.25) == TF_FORMAT);
        CHECK(required == 99 && unchanged(out, sizeof out) && m.releases == 1 &&
              !m.live);
    }
    m = (struct tf_memory){.fail = 1};
    s = (struct tf_conversion){0};
    required = 99;
    memset(out, '@', sizeof out);
    CHECK(tf_format(out, sizeof out, &required, a, scalar, 1u, 'I', 7) ==
          TF_NOMEM);
    CHECK(required == 99 && unchanged(out, sizeof out) && m.acquires == 1 &&
          !m.releases);
    m = (struct tf_memory){0};
    s = (struct tf_conversion){0};
    required = 99;
    CHECK(tf_format(out, sizeof out, &required, a, scalar, 5u) == TF_INVALID);
    CHECK(tf_format(out, sizeof out, &required, a, scalar, 1u, '?') ==
          TF_INVALID);
    CHECK(tf_format(out, sizeof out, &required, a, scalar, 2u, 'I', 7, '?') ==
          TF_INVALID);
    CHECK(required == 99 && unchanged(out, sizeof out) && m.acquires == 0 &&
          s.calls == 1);
    CHECK(tf_format(NULL, 1, &required, a, scalar, 0u) == TF_INVALID);
    CHECK(tf_format(out, 1, NULL, a, scalar, 0u) == TF_INVALID);
    CHECK(tf_format(out, 1, &required, (struct tf_allocator){0}, scalar, 0u) ==
          TF_INVALID);
    CHECK(tf_format(out, 1, &required, a, (struct tf_scalars){0}, 0u) ==
          TF_INVALID);
    CHECK(tf_format(NULL, 0, &required, a, scalar, 0u) == TF_FULL &&
          required == 0);
    CHECK(tf_format(out, 1, &required, a, scalar, 0u) == TF_OK && out[0] == 0 &&
          required == 0);
    CHECK(m.bytes == 1 && m.releases == 1 && !m.live);
    puts(
        "TF_LIFETIME_PASS two_independent_copies helper_borrow early_exit exact_once_cleanup");
    struct tf_arg args[] = {{'I', 7, 0}, {'D', 0, 1.25}};
    m = (struct tf_memory){0};
    s = (struct tf_conversion){0};
    required = 99;
    memset(out, '@', sizeof out);
    CHECK(tf_array(out, sizeof out, &required, a, scalar, args, 1, 2) ==
          TF_INVALID);
    CHECK(tf_array(out, sizeof out, &required, a, scalar, NULL, 2, 2) ==
          TF_INVALID);
    CHECK(required == 99 && unchanged(out, sizeof out) && s.calls == 0);
    CHECK(tf_array(out, 11, &required, a, scalar, args, 2, 2) == TF_FULL &&
          required == 11);
    CHECK(unchanged(out, sizeof out) && m.acquires == 0);
    CHECK(tf_array(out, sizeof out, &required, a, scalar, args, 2, 2) == TF_OK);
    CHECK(strcmp(out, "I=7;D=1.25;") == 0 && required == 11 && m.releases == 1);
    args[1].tag = '?';
    required = 99;
    memset(out, '@', sizeof out);
    CHECK(tf_array(out, sizeof out, &required, a, scalar, args, 2, 2) ==
          TF_INVALID);
    CHECK(required == 99 && unchanged(out, sizeof out));
    CHECK(tf_array(out, sizeof out, &required, a, scalar, NULL, 0, 0) ==
              TF_OK &&
          required == 0 && out[0] == 0);
    args[1].tag = 'D';
    for (unsigned fail = 1; fail <= 4; ++fail)
    {
        m = (struct tf_memory){0};
        s = (struct tf_conversion){.fail_at = fail};
        required = 99;
        memset(out, '@', sizeof out);
        CHECK(tf_array(out, sizeof out, &required, a, scalar, args, 2, 2) ==
              TF_FORMAT);
        CHECK(required == 99 && unchanged(out, sizeof out) &&
              m.releases == (fail > 2) && !m.live);
    }
    m = (struct tf_memory){.fail = 1};
    s = (struct tf_conversion){0};
    required = 99;
    CHECK(tf_array(out, sizeof out, &required, a, scalar, args, 2, 2) ==
          TF_NOMEM);
    CHECK(required == 99 && unchanged(out, sizeof out) && m.acquires == 1 &&
          !m.releases);
    CHECK(tf_array(out, sizeof out, &required, a, scalar, args, 2, 5) ==
          TF_INVALID);
    puts(
        "TF_ARRAY_PASS explicit_available_count tags borrow no_active_union_claim");
    return 0;
}

struct tf_steps
{
    int values[3];
    unsigned fail, gate_fail, used;
    char trace[8];
};

static enum tf_status step(struct tf_steps *s, unsigned i, int *out)
{
    s->trace[s->used++] = (char)('A' + i);
    s->trace[s->used] = 0;
    if (s->fail == i + 1)
        return TF_FORMAT;
    *out = s->values[i];
    return TF_OK;
}

static enum tf_status a_step(void *s, int *v)
{
    return step(s, 0, v);
}

static enum tf_status b_step(void *s, int *v)
{
    return step(s, 1, v);
}

static enum tf_status c_step(void *s, int *v)
{
    return step(s, 2, v);
}

static enum tf_status gate(void *v)
{
    struct tf_steps *s = v;
    s->trace[s->used++] = 'G';
    s->trace[s->used] = 0;
    return s->gate_fail ? TF_NOMEM : TF_OK;
}

static int expressions(void)
{
    unsigned char byte = 77;
    CHECK(tf_narrow_signed(-1, &byte) == TF_SIZE && byte == 77);
    CHECK(tf_narrow_signed((long long)UCHAR_MAX + 1, &byte) == TF_SIZE &&
          byte == 77);
    CHECK(tf_narrow_signed(UCHAR_MAX, &byte) == TF_OK && byte == UCHAR_MAX);
    CHECK(tf_narrow_signed(0, &byte) == TF_OK && byte == 0);
    CHECK(tf_narrow_signed(1, NULL) == TF_INVALID);
    CHECK(tf_narrow_size(SIZE_MAX, &byte) == TF_SIZE && byte == 0);
    CHECK(tf_narrow_size(7, &byte) == TF_OK && byte == 7);
    CHECK((size_t)-1 == SIZE_MAX); /* Valid conversion loses provenance. */
    struct tf_steps s = {{1, 2, 3}, 0, 0, 0, {0}};
    struct tf_progress p = {88, 99};
    CHECK(tf_steps(a_step, b_step, c_step, gate, &s, &p) == TF_OK &&
          p.result == 7);
    CHECK(strcmp(s.trace, "AGBC") == 0 && p.completed == 3);
    s = (struct tf_steps){{1, 2, 3}, 0, 1, 0, {0}};
    p = (struct tf_progress){88, 99};
    CHECK(tf_steps(a_step, b_step, c_step, gate, &s, &p) == TF_NOMEM);
    CHECK(strcmp(s.trace, "AG") == 0 && p.completed == 1 && p.result == 99);
    for (unsigned fail = 1; fail <= 3; ++fail)
    {
        s = (struct tf_steps){{1, 2, 3}, fail, 0, 0, {0}};
        p = (struct tf_progress){88, 99};
        CHECK(tf_steps(a_step, b_step, c_step, gate, &s, &p) == TF_FORMAT);
        CHECK(p.completed == fail - 1 && p.result == 99);
        CHECK(strcmp(s.trace, fail == 1   ? "A"
                              : fail == 2 ? "AGB"
                                          : "AGBC") == 0);
    }
    int values[][3] = {{1, INT_MAX, 1},  {0, INT_MIN, -1}, {0, INT_MAX, 2},
                       {INT_MIN, -1, 1}, {0, INT_MIN, 1},  {0, 0, INT_MIN},
                       {INT_MAX, 0, 2},  {-7, -2, -3}};
    for (unsigned i = 0; i < sizeof values / sizeof values[0]; ++i)
    {
        s = (struct tf_steps){
            {values[i][0], values[i][1], values[i][2]}, 0, 0, 0, {0}};
        p = (struct tf_progress){88, 99};
        enum tf_status status = tf_steps(a_step, b_step, c_step, gate, &s, &p);
        CHECK(status == (i < 4 ? TF_SIZE : TF_OK));
        CHECK(p.completed == 3 && strcmp(s.trace, "AGBC") == 0);
        if (i < 4)
            CHECK(p.result == 99);
        else
            CHECK(p.result == (i == 4   ? INT_MIN
                               : i == 5 ? 0
                               : i == 6 ? INT_MAX
                                        : -1));
    }
    p = (struct tf_progress){88, 99};
    CHECK(tf_steps(NULL, b_step, c_step, gate, &s, &p) == TF_INVALID &&
          p.completed == 88 && p.result == 99);
    puts(
        "TF_STEPS_PASS checked_arithmetic ordered_callbacks fallible_gate partial_progress");
    puts("TF_NARROW_PASS admission_before_conversion negative_provenance_lost");
    return 0;
}

int main(void)
{
    CHECK(setlocale(LC_ALL, "C") != NULL);
    if (formatting() || expressions())
        return 1;
    puts("TF_PASS variadic_expressions");
    return 0;
}
