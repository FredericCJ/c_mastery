#ifndef NUMERICAL_DEPTH_H
#define NUMERICAL_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum
{
    ND_OK,
    ND_BAD,
    ND_RANGE,
    ND_SINGULAR,
    ND_LIMIT,
    ND_STAGNANT
};

enum
{
    ND_NAIVE,
    ND_PAIRWISE,
    ND_KAHAN,
    ND_NEUMAIER
};

int nd_sum(const double *, size_t, unsigned, double *);
int nd_qmul(int16_t, int16_t, unsigned, int16_t *);
int nd_qmac(const int16_t *, const int16_t *, size_t, unsigned, int16_t *);
int nd_quantize(double, unsigned, int16_t *);

struct nd_view
{
    double *data;
    size_t extent, rows, cols, rs, cs;
};

int nd_view(struct nd_view);
int nd_matrix(struct nd_view, struct nd_view, struct nd_view, unsigned);
int nd_scale(struct nd_view, double);
int nd_dot(struct nd_view, struct nd_view, double *);
int nd_interp(const double *, const double *, size_t, double, int, double *);

struct nd_cost
{
    size_t adds, multiplies, divisions, row_moves;
};

int nd_solve(struct nd_view, const double *, size_t, double, double *,
             struct nd_cost *);
int nd_jacobi(struct nd_view, const double *, size_t, double, unsigned,
              double *, unsigned *);
int nd_euler(double, double, double, size_t, double *, size_t);
int nd_iir(double *, const double *, size_t, double, double, double *, size_t);
#define ND_ITEMS 16

struct nd_record
{
    double x, y;
    unsigned id;
    char label[24];
};

struct nd_store
{
    double x[ND_ITEMS], y[ND_ITEMS];
    unsigned id[ND_ITEMS];
    char label[ND_ITEMS][24];
    size_t count;
    unsigned serial;
};

int nd_insert(struct nd_store *, double, double, const char[24], unsigned *);
int nd_remove(struct nd_store *, unsigned);
int nd_reorder(struct nd_store *, const unsigned *, size_t);
int nd_record(const struct nd_store *, unsigned, struct nd_record *);
#endif
