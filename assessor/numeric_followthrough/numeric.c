#include "numeric.h"
#include <float.h>
#include <string.h>

_Static_assert(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024 &&
                   sizeof(double) == 8,
               "selected binary64 profile");

#ifndef NF_MUTANT
#define NF_MUTANT 0
#endif

/* Inputs are accepted only within +/-NF_INPUT_LIMIT, so |got - want| <= 2e100
 * is finite and the subtraction below can neither overflow nor produce NaN. */
#define NF_INPUT_LIMIT 1e100

/* Telemetry budget: absolute floor plus a relative term, both 2^-40.  The
 * floor is what lets a reference of zero admit a 2^-44 discrepancy. */
#define NF_TELEMETRY_ABSOLUTE 0x1p-40
#define NF_TELEMETRY_RELATIVE 0x1p-40

static double magnitude(double x)
{
    return x < 0 ? -x : x;
}

/* A positive range test: NaN fails every comparison, so it is rejected along
 * with the out-of-range values. */
static int in_input_range(double x)
{
    return x >= -NF_INPUT_LIMIT && x <= NF_INPUT_LIMIT;
}

static int policy_is_valid(enum nf_policy policy)
{
    return policy == NF_TELEMETRY || policy == NF_AUDIT;
}

/* Audit compares object representations so that +0.0 and -0.0 differ. */
static int audit_accepts(double want, double got)
{
    return memcmp(&want, &got, sizeof want) == 0;
}

static int telemetry_accepts(double error, double scale)
{
    return error <= NF_TELEMETRY_ABSOLUTE + NF_TELEMETRY_RELATIVE * scale;
}

/* Contract: see numeric.h.  *out is written only on the 0 return; every
 * refusal (-1) leaves it untouched, so the result is published atomically. */
int nf_compare(enum nf_policy policy, double want, double got,
               struct nf_error *out)
{
    if (out == NULL || !policy_is_valid(policy) || !in_input_range(want) ||
        !in_input_range(got))
    {
        return -1;
    }

    const double scale = magnitude(want);
    const double error = magnitude(got - want);
    /* Scaled error divides by max(1, |want|) so it stays defined at zero. */
    const double divisor = scale > 1 ? scale : 1;

    struct nf_error next;
    next.absolute = error;
    next.scaled = error / divisor;
    next.accepted = policy == NF_AUDIT ? audit_accepts(want, got)
                                       : telemetry_accepts(error, scale);
#if NF_MUTANT == 1
    next.accepted = error <= NF_TELEMETRY_RELATIVE * scale;
#elif NF_MUTANT == 2
    next.accepted =
        error <= NF_TELEMETRY_ABSOLUTE + NF_TELEMETRY_RELATIVE * scale;
#endif
    *out = next;
    return 0;
}
