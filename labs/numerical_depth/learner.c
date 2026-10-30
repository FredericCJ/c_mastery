#include "numeric.h"

/*
 * Learner scaffold: deliberately unsolved. Each entry point ignores its
 * arguments and returns ND_BAD, so the public tests reject it until you
 * implement the contract in numeric.h and CONTRACTS.md.
 */

int nd_sum(const double *values, size_t count, unsigned method, double *out)
{
    (void)values;
    (void)count;
    (void)method;
    (void)out;
    return ND_BAD;
}

int nd_qmul(int16_t a, int16_t b, unsigned q, int16_t *out)
{
    (void)a;
    (void)b;
    (void)q;
    (void)out;
    return ND_BAD;
}

int nd_qmac(const int16_t *a, const int16_t *b, size_t count, unsigned q,
            int16_t *out)
{
    (void)a;
    (void)b;
    (void)count;
    (void)q;
    (void)out;
    return ND_BAD;
}

int nd_quantize(double value, unsigned q, int16_t *out)
{
    (void)value;
    (void)q;
    (void)out;
    return ND_BAD;
}

int nd_view(struct nd_view view)
{
    (void)view;
    return ND_BAD;
}

int nd_matrix(struct nd_view a, struct nd_view b, struct nd_view c,
              unsigned method)
{
    (void)a;
    (void)b;
    (void)c;
    (void)method;
    return ND_BAD;
}

int nd_scale(struct nd_view view, double factor)
{
    (void)view;
    (void)factor;
    return ND_BAD;
}

int nd_dot(struct nd_view a, struct nd_view b, double *out)
{
    (void)a;
    (void)b;
    (void)out;
    return ND_BAD;
}

int nd_interp(const double *knots, const double *ordinates, size_t count,
              double query, int clamp, double *out)
{
    (void)knots;
    (void)ordinates;
    (void)count;
    (void)query;
    (void)clamp;
    (void)out;
    return ND_BAD;
}

int nd_solve(struct nd_view a, const double *rhs, size_t n, double tolerance,
             double *out, struct nd_cost *cost)
{
    (void)a;
    (void)rhs;
    (void)n;
    (void)tolerance;
    (void)out;
    (void)cost;
    return ND_BAD;
}

int nd_jacobi(struct nd_view a, const double *rhs, size_t n, double tolerance,
              unsigned limit, double *out, unsigned *steps)
{
    (void)a;
    (void)rhs;
    (void)n;
    (void)tolerance;
    (void)limit;
    (void)out;
    (void)steps;
    return ND_BAD;
}

int nd_euler(double y0, double h, double rate, size_t count, double *out,
             size_t capacity)
{
    (void)y0;
    (void)h;
    (void)rate;
    (void)count;
    (void)out;
    (void)capacity;
    return ND_BAD;
}

int nd_iir(double *state, const double *in, size_t count, double feed,
           double feedback, double *out, size_t capacity)
{
    (void)state;
    (void)in;
    (void)count;
    (void)feed;
    (void)feedback;
    (void)out;
    (void)capacity;
    return ND_BAD;
}

int nd_insert(struct nd_store *store, double x, double y,
              const char label[ND_LABEL_BYTES], unsigned *id)
{
    (void)store;
    (void)x;
    (void)y;
    (void)label;
    (void)id;
    return ND_BAD;
}

int nd_remove(struct nd_store *store, unsigned id)
{
    (void)store;
    (void)id;
    return ND_BAD;
}

int nd_reorder(struct nd_store *store, const unsigned *order, size_t count)
{
    (void)store;
    (void)order;
    (void)count;
    return ND_BAD;
}

int nd_record(const struct nd_store *store, unsigned id, struct nd_record *out)
{
    (void)store;
    (void)id;
    (void)out;
    return ND_BAD;
}
