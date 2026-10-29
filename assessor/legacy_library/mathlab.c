#include "mathlab.h"
#include <errno.h>
#include <fenv.h>
#include <math.h>

enum lm_status lm_observe(double (*f)(double), double x, struct lm_sample *out)
{
    if (!f || !out)
        return LM_ARGUMENT;
#if (defined(__FAST_MATH__) ||                                   \
     (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__)) && \
    !defined(LM_MUTANT_POLICY)
    (void)x;
    return LM_UNSUPPORTED;
#else
    fenv_t saved;
    int saved_errno = errno;
    if (fegetenv(&saved))
    {
        errno = saved_errno;
        return LM_ENVIRONMENT;
    }
    if (feclearexcept(FE_ALL_EXCEPT))
    {
        (void)fesetenv(&saved);
        errno = saved_errno;
        return LM_ENVIRONMENT;
    }
#ifndef LM_MUTANT_ERRNO
    errno = 0;
#endif
    struct lm_sample sample = {0};
    sample.rounding = fegetround();
    if (sample.rounding < 0)
    {
        (void)fesetenv(&saved);
        errno = saved_errno;
        return LM_ENVIRONMENT;
    }
    sample.advertised = math_errhandling;
    sample.value = f(x);
    sample.error = errno;
    sample.exceptions = fetestexcept(FE_ALL_EXCEPT);
#ifndef LM_MUTANT_RESTORE
    if (fesetenv(&saved))
    {
        errno = saved_errno;
        return LM_ENVIRONMENT;
    }
#endif
    errno = saved_errno;
    *out = sample;
    return LM_OK;
#endif
}
