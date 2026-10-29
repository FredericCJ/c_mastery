#include "float_contract.h"
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef __clang__
#pragma STDC FENV_ACCESS ON
#endif
#define CHECK(x)                                  \
    do                                            \
    {                                             \
        if (!(x))                                 \
        {                                         \
            fprintf(stderr, "FF_CHECK %s\n", #x); \
            return 1;                             \
        }                                         \
    } while (0)
static int calls, fail_at, restore_calls;

static int fail_save(fenv_t *p)
{
    (void)p;
    return 1;
}

static int fail_hold(fenv_t *p)
{
    int rc = feholdexcept(p);
    return rc ? rc : 1;
}

static int fail_mode(int mode)
{
    return ++calls == fail_at ? 1 : fesetround(mode);
}

static int count_restore(const fenv_t *p)
{
    restore_calls++;
    return fesetenv(p);
}

static int fail_restore(const fenv_t *p)
{
    (void)p;
    restore_calls++;
    return 1;
}

int main(void)
{
    CHECK(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024);
    fenv_t original;
    CHECK(fegetenv(&original) == 0 && fesetround(FE_TONEAREST) == 0);
    double out = 917, tri[] = {1e16, 1, -1e16}, bad[] = {1, NAN};
    CHECK(ff_sum(tri, 3, FF_NEUMAIER, &out) == FF_OK && out == 1);
    CHECK(ff_sum(tri, 3, FF_NAIVE, &out) == FF_OK && out == 0);
    CHECK(ff_sum(tri, 3, FF_KAHAN, &out) == FF_OK && out == 0);
    out = 917;
    CHECK(ff_sum(bad, 2, FF_NEUMAIER, &out) == FF_RANGE && out == 917);
    CHECK(ff_sum(NULL, 0, FF_NEUMAIER, &out) == FF_OK && out == 0);
    CHECK(ff_sum(tri, 4097, FF_NAIVE, &out) == FF_RANGE);
    double small[] = {0x1p-101};
    CHECK(ff_sum(small, 1, FF_NAIVE, &out) == FF_RANGE);
    CHECK(ff_dot(tri, (double[]){1, 1, 1}, 3, &out) == FF_OK && out == 0);
    int eq = 917;
    CHECK(ff_compare(0, 1e-6, 2e-6, 0, 0, &eq) == FF_OK && eq);
    CHECK(ff_compare(1e100, nextafter(1e100, INFINITY), 0, 1e-14, 0, &eq) ==
              FF_OK &&
          eq);
    CHECK(ff_compare(DBL_MAX, -DBL_MAX, DBL_MAX, 1, 0, &eq) == FF_OK && !eq);
    CHECK(ff_compare(-0.0, 0.0, 0, 0, 1, &eq) == FF_OK && !eq);
    CHECK(ff_compare(-0.0, 0.0, 0, 0, 0, &eq) == FF_OK && eq);
    eq = 917;
    CHECK(ff_compare(NAN, 0, 0, 0, 0, &eq) == FF_RANGE && eq == 917);
    uint64_t distance = 917;
    CHECK(ff_ulps(-DBL_TRUE_MIN, DBL_TRUE_MIN, &distance) == FF_OK &&
          distance == 2);
    CHECK(ff_ulps(-0.0, 0.0, &distance) == FF_OK && distance == 0);
    CHECK(ff_ulps(1, nextafter(1, INFINITY), &distance) == FF_OK &&
          distance == 1);
    CHECK(ff_ulps(-1, nextafter(-1, -INFINITY), &distance) == FF_OK &&
          distance == 1);
    distance = 917;
    CHECK(ff_ulps(NAN, 0, &distance) == FF_RANGE && distance == 917);
    CHECK(feclearexcept(FE_ALL_EXCEPT) == 0 &&
          feraiseexcept(FE_DIVBYZERO) == 0 && fesetround(FE_DOWNWARD) == 0);
    int flags = fetestexcept(FE_ALL_EXCEPT);
    errno = EDOM;
    struct ff_round round;
    CHECK(ff_round_probe(1, 0x1p-53, &round) == FF_OK && round.count == 4);
    CHECK(fegetround() == FE_DOWNWARD && fetestexcept(FE_ALL_EXCEPT) == flags &&
          errno == EDOM);
    for (size_t i = 0; i < round.count; i++)
        CHECK(round.values[i] ==
              (round.modes[i] == FE_UPWARD ? 0x1.0000000000001p0 : 1));
    for (int failure = 0; failure < 7; failure++)
    {
        struct ff_env_ops ops = {fegetenv, feholdexcept, fail_mode,
                                 count_restore};
        struct ff_round sentinel = {{917}, {917}, 917};
        unsigned char before[sizeof sentinel];
        memcpy(before, &sentinel, sizeof sentinel);
        calls = 0;
        restore_calls = 0;
        fail_at = failure;
        if (failure == 0)
            ops.save = fail_save;
        if (failure == 5)
            ops.hold = fail_hold;
        if (failure == 6)
            ops.restore = fail_restore;
        CHECK(ff_probe_ops(1, 0x1p-53, &sentinel, &ops) ==
              (failure == 6 ? FF_RESTORE : FF_ENV));
        CHECK(memcmp(&sentinel, before, sizeof sentinel) == 0 && errno == EDOM);
        CHECK(restore_calls == (failure == 0 ? 0 : 1));
        if (failure != 6)
            CHECK(fegetround() == FE_DOWNWARD &&
                  fetestexcept(FE_ALL_EXCEPT) == flags);
        else
            CHECK(fesetenv(&original) == 0);
    }
    CHECK(fesetenv(&original) == 0);
    puts(
        "FF_PASS sum compensation rejection comparison ulps environment restoration_failures");
    return 0;
}
