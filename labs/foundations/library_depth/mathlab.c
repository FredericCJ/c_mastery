#include "mathlab.h"

enum lm_status lm_observe(double (*f)(double), double x, struct lm_sample *out)
{
    (void)f;
    (void)x;
    (void)out;
    return LM_NOT_IMPLEMENTED;
}
