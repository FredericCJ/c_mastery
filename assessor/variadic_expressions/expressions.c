#include "expressions.h"
#include <limits.h>
#ifndef TF_MUTANT
#define TF_MUTANT 0
#endif
static int multiply(int a, int b, int *out)
{
    if (a > 0 ? (b > 0 ? a > INT_MAX / b : b < INT_MIN / a)
              : (b > 0 ? a < INT_MIN / b : a != 0 && b < INT_MAX / a))
        return 0;
    *out = a * b;
    return 1;
}

static int add(int a, int b, int *out)
{
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
        return 0;
    *out = a + b;
    return 1;
}

enum tf_status tf_steps(tf_step_fn a, tf_step_fn b, tf_step_fn c,
                        tf_gate_fn gate, void *context, struct tf_progress *p)
{
    if (!a || !b || !c || !gate || !p)
        return TF_INVALID;
    p->completed = 0;
    int x, y, z, product, total;
    enum tf_status status = (TF_MUTANT == 4 ? b : a)(context, &x);
    if (status != TF_OK)
        return status;
    p->completed = 1;
    status = gate(context);
    if (status != TF_OK && TF_MUTANT != 5)
        return status;
    status = (TF_MUTANT == 4 ? a : b)(context, &y);
    if (status != TF_OK)
        return status;
    p->completed = 2;
    status = c(context, &z);
    if (status != TF_OK)
        return status;
    p->completed = 3;
    if (!multiply(y, z, &product) || !add(x, product, &total))
        return TF_SIZE;
    p->result = total;
    return TF_OK;
}

enum tf_status tf_narrow_signed(long long value, unsigned char *out)
{
    if (!out)
        return TF_INVALID;
    if (TF_MUTANT != 6 && (value < 0 || value > UCHAR_MAX))
        return TF_SIZE;
    *out = (unsigned char)value;
    return TF_OK;
}

enum tf_status tf_narrow_size(size_t value, unsigned char *out)
{
    if (!out)
        return TF_INVALID;
    if (value > UCHAR_MAX)
        return TF_SIZE;
    *out = (unsigned char)value;
    return TF_OK;
}
