#include "higher_order.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "MFT_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                            \
        }                                                       \
    } while (0)

struct mf_context
{
    unsigned calls;
    int full;
};

static int eval_fail(void *opaque, he_int input, he_int *output)
{
    struct mf_context *context = opaque;
    ++context->calls;
    *output = input + 1; /* The probe admits only input 7. */
    return HO_FAILED;
}

static int print_body(void *opaque, struct hp_int input, struct hp_int *output)
{
    struct mf_context *context = opaque;
    ++context->calls;
    CHECK(input.length == 1 && input.data[0] == 'x');
    if (!context->full)
    {
        *output = (struct hp_int){{'7', 0}, 1};
        return HO_FAILED;
    }
    struct hp_int result = {{0}, 127};
    for (unsigned i = 0; i < 127; ++i)
    {
        result.data[i] = 'q';
    }
    *output = result;
    return HO_OK;
}

static int same(const char *left, const char *right)
{
    for (unsigned i = 0; i < 128; ++i)
    {
        if (left[i] != right[i])
        {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    struct he_ops eval = ho_eval_ops();
    struct hp_ops render = ho_print_ops();
    CHECK(eval.lam && eval.app && render.lam && render.app);
    struct mf_context e = {0, 0};
    struct he_fun ef;
    CHECK(eval.lam(eval_fail, &e, &ef) == HO_OK && e.calls == 0);
    he_int output = 91;
    int status = eval.app(ef, 7, &output);
    CHECK(status == HO_FAILED && output == 91 && e.calls == 1);
    printf("eval %d %d %u\n", status, output, e.calls);
    for (int full = 0; full <= 1; ++full)
    {
        struct mf_context p = {0, full};
        struct hp_fun sentinel = {{0}, 6};
        for (unsigned i = 0; i < 128; ++i)
        {
            sentinel.data[i] = 's';
        }
        sentinel.data[6] = 0;
        struct hp_fun value = sentinel;
        status = render.lam(print_body, &p, &value);
        CHECK(status == (full ? HO_FULL : HO_FAILED));
        CHECK(value.length == sentinel.length &&
              same(value.data, sentinel.data) && p.calls == 1);
        printf("lambda %d %d %zu %u\n", full, status, value.length, p.calls);
    }
    struct hp_fun full = {{0}, 127};
    for (unsigned i = 0; i < 127; ++i)
    {
        full.data[i] = 'q';
    }
    struct hp_int argument = {{'1', 0}, 1};
    struct hp_int sentinel = {{'9', 0}, 1}, rendered = sentinel;
    status = render.app(full, argument, &rendered);
    CHECK(status == HO_FULL && rendered.length == sentinel.length &&
          same(rendered.data, sentinel.data));
    printf("application %d %zu %c\n", status, rendered.length,
           rendered.data[0]);
    return 0;
}
