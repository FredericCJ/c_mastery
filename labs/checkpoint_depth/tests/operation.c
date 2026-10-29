#include "checkpoint_operation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "CP_CHECK: %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

struct cpx_context
{
    struct cp_operation *operation;
    unsigned allocations, releases, reports;
    int fail, cancelled;
    unsigned char expected[128];
};

static void *allocate(void *c, size_t n)
{
    struct cpx_context *x = c;
    if (x->fail)
    {
        return 0;
    }
    ++x->allocations;
    return malloc(n);
}

static void release(void *c, void *p)
{
    struct cpx_context *x = c;
    ++x->releases;
    free(p);
}

static int report(void *c, unsigned gen, const unsigned char *p, size_t n,
                  int cancelled)
{
    struct cpx_context *x = c;
    ++x->reports;
    x->cancelled = cancelled;
    CHECK(n == sizeof x->expected && !memcmp(p, x->expected, n));
    unsigned unchanged = 99;
    CHECK(cpo_begin(x->operation, p, n, &unchanged) == CPO_BUSY &&
          unchanged == 99);
    CHECK(cpo_cancel(x->operation, gen) == CPO_STALE &&
          cpo_complete(x->operation, gen, 1) == CPO_STALE);
    return 1;
}

struct cpx_completion
{
    struct cp_operation *operation;
    unsigned generation;
    enum cpo_status result;
};

static void *complete_thread(void *value)
{
    struct cpx_completion *c = value;
    c->result = cpo_complete(c->operation, c->generation, 1);
    return 0;
}

void cp_test_operation(void)
{
    struct cp_operation operation;
    struct cpx_context context = {.operation = &operation};
    for (size_t i = 0; i < sizeof context.expected; ++i)
    {
        context.expected[i] = (unsigned char)(i ^ 0xa5);
    }
    unsigned char input[128];
    memcpy(input, context.expected, sizeof input);
    CHECK(cpo_init(&operation, 2, &context, allocate, release, report) ==
          CPO_OK);
    unsigned generation = 99;
    context.fail = 1;
    CHECK(cpo_begin(&operation, input, sizeof input, &generation) ==
              CPO_NOMEM &&
          generation == 99 && !context.allocations);
    context.fail = 0;
    CHECK(cpo_begin(&operation, input, sizeof input, &generation) == CPO_OK &&
          generation == 1);
    memset(input, 0, sizeof input);
    CHECK(!memcmp(operation.bytes, context.expected, sizeof input));
    CHECK(cpo_cancel(&operation, generation) == CPO_PENDING &&
          cpo_cancel(&operation, generation) == CPO_PENDING);
    CHECK(cpo_complete(&operation, generation, 0) == CPO_PENDING &&
          context.releases == 0 && context.reports == 0 && operation.bytes);
    CHECK(cpo_destroy(&operation) == CPO_PENDING);
    struct cpx_completion c = {&operation, generation, CPO_ERROR};
    pthread_t worker;
    CHECK(!pthread_create(&worker, 0, complete_thread, &c) &&
          !pthread_join(worker, 0));
    CHECK(c.result == CPO_OK && context.releases == 1 && context.reports == 1 &&
          context.cancelled && operation.report_failed == 1);
    CHECK(cpo_complete(&operation, generation, 1) == CPO_STALE &&
          context.releases == 1);
    unsigned next;
    CHECK(cpo_begin(&operation, context.expected, sizeof input, &next) ==
              CPO_OK &&
          next == 2);
    CHECK(cpo_complete(&operation, generation, 1) == CPO_STALE &&
          operation.state == CPO_ACTIVE && context.releases == 1);
    CHECK(cpo_complete(&operation, next, 1) == CPO_OK && !context.cancelled &&
          context.allocations == 2 && context.releases == 2 &&
          context.reports == 2);
    CHECK(cpo_begin(&operation, context.expected, sizeof input, &next) ==
              CPO_RETIRED &&
          next == 2);
    CHECK(cpo_destroy(&operation) == CPO_OK);
}
