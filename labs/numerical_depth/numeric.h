#ifndef NUMERICAL_DEPTH_H
#define NUMERICAL_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum { ND_OK, ND_BAD, ND_RANGE, ND_SINGULAR, ND_LIMIT, ND_STAGNANT };
enum { ND_NAIVE, ND_PAIRWISE, ND_KAHAN, ND_NEUMAIER };
int nd_sum(const double *, size_t, unsigned, double *);
int nd_qmul(int16_t, int16_t, unsigned, int16_t *);
int nd_qmac(const int16_t *, const int16_t *, size_t, unsigned, int16_t *);
int nd_quantize(double, unsigned, int16_t *);
typedef struct {
  double *data;
  size_t extent, rows, cols, rs, cs;
} NDView;
int nd_view(NDView);
int nd_matrix(NDView, NDView, NDView, unsigned);
int nd_scale(NDView, double);
int nd_dot(NDView, NDView, double *);
int nd_interp(const double *, const double *, size_t, double, int, double *);
typedef struct {
  size_t adds, multiplies, divisions, row_moves;
} NDCost;
int nd_solve(NDView, const double *, size_t, double, double *, NDCost *);
int nd_jacobi(NDView, const double *, size_t, double, unsigned, double *,
              unsigned *);
int nd_euler(double, double, double, size_t, double *, size_t);
int nd_iir(double *, const double *, size_t, double, double, double *, size_t);
#define ND_ITEMS 16
typedef struct {
  double x, y;
  unsigned id;
  char label[24];
} NDRecord;
typedef struct {
  double x[ND_ITEMS], y[ND_ITEMS];
  unsigned id[ND_ITEMS];
  char label[ND_ITEMS][24];
  size_t count;
  unsigned serial;
} NDStore;
int nd_insert(NDStore *, double, double, const char[24], unsigned *);
int nd_remove(NDStore *, unsigned);
int nd_reorder(NDStore *, const unsigned *, size_t);
int nd_record(const NDStore *, unsigned, NDRecord *);
#endif
