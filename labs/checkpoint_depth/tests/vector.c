#include "checkpoint_vector.h"
#include <math.h>
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

struct cpx_fake
{
    uint32_t flags;
    uint64_t state;
    int cpuid, calls;
};

static int fake_leaf(void *v, uint32_t *f)
{
    struct cpx_fake *p = v;
    *f = p->flags;
    return p->cpuid;
}

static uint64_t fake_state(void *v)
{
    struct cpx_fake *p = v;
    ++p->calls;
    return p->state;
}

void cp_test_dispatch(void)
{
    const uint32_t avx = UINT32_C(1) << 28, os = UINT32_C(1) << 27;
    for (unsigned flags = 0; flags < 4; ++flags)
        for (unsigned state = 0; state < 8; ++state)
        {
            struct cpx_fake f = {(flags & 1 ? avx : 0) | (flags & 2 ? os : 0),
                                 state, 1, 0};
            struct cpv_probe p = {&f, fake_leaf, fake_state};
            CHECK(cpv_detect(p) == (flags == 3 && (state & 6) == 6));
            CHECK(f.calls == (flags == 3));
        }
    struct cpx_fake f = {avx | os, 6, 0, 0};
    struct cpv_probe p = {&f, fake_leaf, fake_state};
    CHECK(!cpv_detect(p) && !f.calls);
    struct cpv_plan plan;
    CHECK(cpv_plan(&plan, CPV_AUTO) == CPN_OK);
    CHECK(plan.backend == (cpv_available() ? CPV_AVX : CPV_SCALAR));
    struct cpv_plan before = {CPV_COMPILER};
    plan = before;
    CHECK(cpv_plan(&plan, (enum cpv_backend)99) == CPN_INVALID &&
          plan.backend == before.backend);
    if (!cpv_available())
        CHECK(cpv_plan(&plan, CPV_AVX) == CPN_UNSUPPORTED &&
              plan.backend == before.backend);
    printf("CP_CAPS: avx_usable=%d\n", cpv_available());
}

void cp_test_vector(void)
{
    for (int backend = CPV_SCALAR; backend <= CPV_COMPILER; ++backend)
    {
        struct cpv_plan plan;
        enum cpn_status made = cpv_plan(&plan, (enum cpv_backend)backend);
        if (made == CPN_UNSUPPORTED)
            continue;
        CHECK(made == CPN_OK);
        for (size_t n = 0; n <= CPN_BLOCK; ++n)
            for (size_t offset = 0; offset < 4; ++offset)
            {
                double storage[CPN_BLOCK + 8], expected[CPN_BLOCK],
                    out[CPN_BLOCK + 2];
                double *input = storage + offset;
                for (size_t i = 0; i < n; ++i)
                    input[i] = (double)((int)(i % 19) - 9) / 8.0;
                struct cpn_double reference = {4, -8}, actual = reference;
                CHECK(cpn_double(&reference, input, n, expected, n) == CPN_OK);
                out[0] = 71;
                out[n + 1] = 72;
                CHECK(cpv_run(&plan, &actual, input, n, out + 1, n) == CPN_OK);
                CHECK(!memcmp(expected, out + 1, n * sizeof(double)) &&
                      actual.previous == reference.previous &&
                      actual.older == reference.older);
                CHECK(out[0] == 71 && out[n + 1] == 72);
                /* Both overlap directions are part of the public staged contract. */
                actual = (struct cpn_double){4, -8};
                CHECK(cpv_run(&plan, &actual, input, n, input + 1, n) ==
                          CPN_OK &&
                      !memcmp(expected, input + 1, n * sizeof(double)));
                for (size_t i = 0; i < n; ++i)
                    input[i + 1] = (double)((int)(i % 19) - 9) / 8.0;
                actual = (struct cpn_double){4, -8};
                CHECK(cpv_run(&plan, &actual, input + 1, n, input, n) ==
                          CPN_OK &&
                      !memcmp(expected, input, n * sizeof(double)));
            }
        double input[] = {-4.0 * 9007199254740992.0, 2.0 * 9007199254740992.0,
                          4, 8, 16},
               out[5];
        for (size_t split = 0; split <= 5; ++split)
        {
            struct cpn_double s = {0, 0};
            CHECK(cpv_run(&plan, &s, input, split, out, split) == CPN_OK);
            CHECK(cpv_run(&plan, &s, input + split, 5 - split, out + split,
                          5 - split) == CPN_OK);
            CHECK(out[2] == 0 && s.previous == 16 && s.older == 8);
        }
        struct cpn_double s = {4, 8};
        out[0] = 123;
        CHECK(cpv_run(&plan, &s, input, 5, out, 4) == CPN_CAPACITY &&
              out[0] == 123 && s.previous == 4);
        input[4] = NAN;
        CHECK(cpv_run(&plan, &s, input, 5, out, 5) == CPN_INVALID &&
              out[0] == 123 && s.previous == 4);
    }
}
