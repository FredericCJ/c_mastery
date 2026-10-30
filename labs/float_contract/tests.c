#include "float_contract.h"
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef __clang__
#pragma STDC FENV_ACCESS ON
#endif
/* The expressions inside CHECK are matched as text by the mutation campaign,
 * so their spelling is part of the test contract. */
#define CHECK(x)                                  \
    do                                            \
    {                                             \
        if (!(x))                                 \
        {                                         \
            fprintf(stderr, "FF_CHECK %s\n", #x); \
            return 1;                             \
        }                                         \
    } while (0)

/* Number of environment-failure scenarios in check_injected_failures(): one
 * failing save, four failing mode changes, one failing hold, one failing
 * restore. */
#define INJECTED_FAILURE_COUNT 7

/* State shared with the injected environment operations below. */
static int calls;
static int fail_at;
static int restore_calls;
static fenv_t original;

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

/* Fails the fail_at-th call and behaves like fesetround otherwise. */
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

/* Summation methods, admission limits and the dot product. 1e16 + 1 - 1e16
 * separates compensated from plain summation: the 1 is below the spacing of
 * doubles near 1e16 and is lost unless an error term carries it. */
static int check_sums(void)
{
    double out = 917;
    double tri[] = {1e16, 1, -1e16};
    double bad[] = {1, NAN};
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
    return 0;
}

/* Tolerance comparison: absolute, relative, overflow-prone and signed zero. */
static int check_compare(void)
{
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
    return 0;
}

/* Distance in units in the last place across zero, subnormals and sign. */
static int check_ulps(void)
{
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
    return 0;
}

/* The probe must leave rounding mode, exception flags and errno exactly as the
 * caller had them, while reporting every mode's result. The caller's state is
 * deliberately unusual (round downward, FE_DIVBYZERO raised, errno = EDOM) so
 * that a probe which resets to defaults would be noticed. On success
 * *flags_out receives the exception flags the caller had set. */
static int check_probe_preserves_environment(int *flags_out)
{
    CHECK(feclearexcept(FE_ALL_EXCEPT) == 0 &&
          feraiseexcept(FE_DIVBYZERO) == 0 && fesetround(FE_DOWNWARD) == 0);
    int flags = fetestexcept(FE_ALL_EXCEPT);
    *flags_out = flags;
    errno = EDOM;
    struct ff_round round;
    CHECK(ff_round_probe(1, 0x1p-53, &round) == FF_OK && round.count == 4);
    CHECK(fegetround() == FE_DOWNWARD && fetestexcept(FE_ALL_EXCEPT) == flags &&
          errno == EDOM);
    for (size_t i = 0; i < round.count; i++)
    {
        /* 1 + 2^-53 is a tie between 1 and the next double: only rounding
         * upward moves it. */
        CHECK(round.values[i] ==
              (round.modes[i] == FE_UPWARD ? 0x1.0000000000001p0 : 1));
    }
    return 0;
}

/* Inject a failure into each environment operation in turn. Every scenario must
 * publish nothing, preserve errno, call restore exactly once (not at all when
 * save itself failed) and, except for a failing restore, leave the caller's
 * rounding mode and flags intact. */
static int check_injected_failures(int flags)
{
    for (int failure = 0; failure < INJECTED_FAILURE_COUNT; failure++)
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
        {
            ops.save = fail_save;
        }
        if (failure == 5)
        {
            ops.hold = fail_hold;
        }
        if (failure == 6)
        {
            ops.restore = fail_restore;
        }
        CHECK(ff_probe_ops(1, 0x1p-53, &sentinel, &ops) ==
              (failure == 6 ? FF_RESTORE : FF_ENV));
        CHECK(memcmp(&sentinel, before, sizeof sentinel) == 0 && errno == EDOM);
        CHECK(restore_calls == (failure == 0 ? 0 : 1));
        if (failure != 6)
        {
            CHECK(fegetround() == FE_DOWNWARD &&
                  fetestexcept(FE_ALL_EXCEPT) == flags);
        }
        else
        {
            /* The injected restore did nothing, so put the original
             * environment back ourselves before the next scenario. */
            CHECK(fesetenv(&original) == 0);
        }
    }
    return 0;
}

int main(void)
{
    CHECK(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024);
    CHECK(fegetenv(&original) == 0 && fesetround(FE_TONEAREST) == 0);

    int flags = 0;
    if (check_sums() || check_compare() || check_ulps() ||
        check_probe_preserves_environment(&flags) ||
        check_injected_failures(flags))
    {
        return 1;
    }
    CHECK(fesetenv(&original) == 0);
    puts(
        "FF_PASS sum compensation rejection comparison ulps environment restoration_failures");
    return 0;
}
