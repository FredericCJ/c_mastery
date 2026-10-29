#include "errors.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "ER_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int optional(void)
{
    uint32_t out = 91;
    er_result r;
    CHECK(er_optional_pack(0, 1, &out) == ER_RANGE && out == 91);
    CHECK(er_optional_pack(17, 0, &out) == ER_OK && out == 0);
    CHECK(er_optional_pack(UINT32_MAX, 1, &out) == ER_OK && out == UINT32_MAX);
    CHECK(er_optional_pack(1, 2, &out) == ER_INVALID && out == UINT32_MAX);
    r = er_optional_unpack(0);
    CHECK(r.tag == ER_ABSENT);
    r = er_optional_unpack(42);
    CHECK(r.tag == ER_OK && r.as.value == 42);
    CHECK(er_error(ER_OK, 0).tag == ER_INVALID);
    return 0;
}

static int propagation(void)
{
    const char *cases[] = {"0:0", "1:65534", "65536:0", "1:2x",
                           "1:",  ":1",      "1:65535", ""};
    er_code tags[] = {ER_OK,      ER_OK,      ER_RANGE, ER_INVALID,
                      ER_INVALID, ER_INVALID, ER_RANGE, ER_INVALID};
    size_t offsets[] = {0, 0, 4, 3, 2, 0, 7, 0};
    size_t i;
    for (i = 0; i < 8; ++i)
    {
        uint32_t out = 99;
        er_result r = er_parse_pair((const unsigned char *)cases[i],
                                    strlen(cases[i]), &out);
        CHECK(r.tag == tags[i]);
        if (r.tag == ER_OK)
            CHECK(out == (i == 0 ? 0 : 65535));
        else
            CHECK(out == 99 && r.as.offset == offsets[i]);
    }
    return 0;
}

typedef struct
{
    size_t amount;
    er_code code;
    unsigned calls;
} sink_state;

static er_progress sink(void *context, const unsigned char *bytes,
                        size_t length)
{
    sink_state *s = context;
    er_progress p = {s->amount, s->code};
    (void)bytes;
    (void)length;
    ++s->calls;
    return p;
}

static int progress(void)
{
    const unsigned char input[] = {1, 2, 3, 4};
    sink_state s = {2, ER_AGAIN, 0};
    er_progress p;
    p = er_transfer(input, 4, 0, sink, &s);
    CHECK(p.next == 2 && p.code == ER_AGAIN && s.calls == 1);
    s.amount = 1;
    s.code = ER_IO;
    p = er_transfer(input, 4, 2, sink, &s);
    CHECK(p.next == 3 && p.code == ER_IO);
    s.amount = 2;
    p = er_transfer(input, 4, 3, sink, &s);
    CHECK(p.next == 3 && p.code == ER_CONTRACT);
    s.amount = 0;
    s.code = ER_OK;
    p = er_transfer(input, 4, 0, sink, &s);
    CHECK(p.next == 0 && p.code == ER_CONTRACT);
    s.calls = 0;
    p = er_transfer(NULL, 0, 0, sink, &s);
    CHECK(p.next == 0 && p.code == ER_OK && s.calls == 0);
    return 0;
}

static uint32_t increment(uint32_t x, void *ctx)
{
    ++*(unsigned *)ctx;
    return x + 1;
}

static er_result reject(uint32_t x, void *ctx)
{
    ++*(unsigned *)ctx;
    return er_error(ER_RANGE, x);
}

static int composition(void)
{
    unsigned calls = 0;
    er_result r;
    r = er_map(er_error(ER_IO, 7), increment, &calls);
    CHECK(r.tag == ER_IO && r.as.offset == 7 && calls == 0);
    r = er_map(er_ok(4), increment, &calls);
    CHECK(r.tag == ER_OK && r.as.value == 5 && calls == 1);
    r = er_then(r, reject, &calls);
    CHECK(r.tag == ER_RANGE && r.as.offset == 5 && calls == 2);
    r = er_then(r, reject, &calls);
    CHECK(r.tag == ER_RANGE && calls == 2);
    return 0;
}

static int validation(void)
{
    er_config bad = {-1, 1025, 0}, out = {7, 8, 9}, good = {150, 1024, 1};
    unsigned errors[3] = {99, 99, 99};
    er_validation r;
    r = er_validate(bad, &out, errors, 2);
    CHECK(r.total == 3 && r.stored == 2 && r.truncated && errors[0] == 1 &&
          errors[1] == 2 && errors[2] == 99);
    CHECK(out.age == 7 && out.quota == 8 && out.denominator == 9);
    r = er_validate(bad, &out, NULL, 0);
    CHECK(r.total == 3 && r.stored == 0 && r.truncated);
    r = er_validate(good, &out, errors, 3);
    CHECK(r.total == 0 && r.stored == 0 && !r.truncated && out.age == 150);
    return 0;
}

typedef struct
{
    unsigned *trace;
    unsigned value;
} cleanup_context;

static void cleanup(void *p)
{
    cleanup_context *c = p;
    *c->trace = *c->trace * 10 + c->value;
}

static er_result throwing(er_frame *f, void *context)
{
    cleanup_context *c = context;
    if (er_defer(f, cleanup, c) != ER_OK ||
        er_defer(f, cleanup, c + 1) != ER_OK)
        return er_error(ER_INVALID, 0);
    (void)er_raise(f, ER_THROWN);
    return er_ok(99);
}

static er_result returning(er_frame *f, void *context)
{
    if (er_defer(f, cleanup, context) != ER_OK)
        return er_error(ER_INVALID, 0);
    return er_ok(9);
}

static int boundary(void)
{
    er_frame frame = {0};
    unsigned trace = 0;
    cleanup_context contexts[2] = {{&trace, 1}, {&trace, 2}};
    er_result r = er_run(&frame, throwing, contexts);
    CHECK(r.tag == ER_THROWN && trace == 21 && !frame.active &&
          frame.count == 0);
    CHECK(er_raise(&frame, ER_THROWN) == ER_INVALID);
    trace = 0;
    r = er_run(&frame, returning, contexts);
    CHECK(r.tag == ER_OK && r.as.value == 9 && trace == 1);
    return 0;
}

static int arithmetic(void)
{
    uint32_t out = 77, a, b;
    int order = 9;
    CHECK(er_add16(65535, 1, ER_CHECKED, &out) == ER_RANGE && out == 77);
    CHECK(er_add16(65535, 1, ER_WRAPPING, &out) == ER_OK && out == 0);
    CHECK(er_add16(65535, 1, ER_SATURATING, &out) == ER_OK && out == 65535);
    CHECK(er_serial16(32768, 0, &order) == ER_RANGE && order == 9);
    CHECK(er_serial16(0, 65535, &order) == ER_OK && order == 1);
    CHECK(er_serial16(65535, 0, &order) == ER_OK && order == -1);
    for (a = 0; a < 256; ++a)
        for (b = 0; b < 256; ++b)
        {
            CHECK(er_add16(a, b, ER_CHECKED, &out) == ER_OK && out == a + b);
        }
    return 0;
}

static int contracts(void)
{
    int value = 90;
    size_t reports = 0;
    CHECK(er_contract_counter(ER_ENFORCE, 20, &value, &reports) ==
              ER_CONTRACT &&
          value == 90 && reports == 1);
    CHECK(er_contract_counter(ER_OBSERVE, 20, &value, &reports) == ER_OK &&
          value == 100 && reports == 2);
    CHECK(er_contract_counter(ER_OFF, 20, &value, &reports) == ER_OK &&
          value == 100 && reports == 2);
    reports = SIZE_MAX;
    value = 90;
    CHECK(er_contract_counter(ER_ENFORCE, 20, &value, &reports) == ER_RANGE &&
          value == 90 && reports == SIZE_MAX);
    return 0;
}

static int refinement(void)
{
    er_fraction r = {91, 92};
    CHECK(er_refine(3, 0, &r) == ER_INVALID && r.numerator == 91 &&
          r.denominator == 92);
    CHECK(er_refine(6, 8, &r) == ER_OK && r.numerator == 3 &&
          r.denominator == 4);
    CHECK(er_refine(0, 13, &r) == ER_OK && r.numerator == 0 &&
          r.denominator == 1);
    CHECK(er_refine(UINT32_MAX, UINT32_MAX, &r) == ER_OK && r.numerator == 1 &&
          r.denominator == 1);
    return 0;
}

int main(int argc, char **argv)
{
    const char *names[] = {"optional",    "propagation", "progress",
                           "composition", "validation",  "boundary",
                           "arithmetic",  "contracts",   "refinement"};
    int (*tests[])(void) = {optional,    propagation, progress,
                            composition, validation,  boundary,
                            arithmetic,  contracts,   refinement};
    size_t i;
    if (argc != 2)
        return 2;
    for (i = 0; i < 9; ++i)
        if (strcmp(argv[1], names[i]) == 0)
            return tests[i]();
    return 2;
}
