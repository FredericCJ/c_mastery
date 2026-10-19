#include "numeric.h"
/* Deliberately unsolved learner entry points. */
int nd_sum(const double *arg0, size_t arg1, unsigned arg2, double *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return ND_BAD;
}
int nd_qmul(int16_t arg0, int16_t arg1, unsigned arg2, int16_t *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return ND_BAD;
}
int nd_qmac(const int16_t *arg0, const int16_t *arg1, size_t arg2,
            unsigned arg3, int16_t *arg4) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  return ND_BAD;
}
int nd_quantize(double arg0, unsigned arg1, int16_t *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return ND_BAD;
}
int nd_view(NDView arg0) {
  (void)arg0;
  return ND_BAD;
}
int nd_matrix(NDView arg0, NDView arg1, NDView arg2, unsigned arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return ND_BAD;
}
int nd_scale(NDView arg0, double arg1) {
  (void)arg0;
  (void)arg1;
  return ND_BAD;
}
int nd_dot(NDView arg0, NDView arg1, double *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return ND_BAD;
}
int nd_interp(const double *arg0, const double *arg1, size_t arg2, double arg3,
              int arg4, double *arg5) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  return ND_BAD;
}
int nd_solve(NDView arg0, const double *arg1, size_t arg2, double arg3,
             double *arg4, NDCost *arg5) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  return ND_BAD;
}
int nd_jacobi(NDView arg0, const double *arg1, size_t arg2, double arg3,
              unsigned arg4, double *arg5, unsigned *arg6) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  (void)arg6;
  return ND_BAD;
}
int nd_euler(double arg0, double arg1, double arg2, size_t arg3, double *arg4,
             size_t arg5) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  return ND_BAD;
}
int nd_iir(double *arg0, const double *arg1, size_t arg2, double arg3,
           double arg4, double *arg5, size_t arg6) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  (void)arg6;
  return ND_BAD;
}
int nd_insert(NDStore *arg0, double arg1, double arg2, const char arg3[24],
              unsigned *arg4) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  return ND_BAD;
}
int nd_remove(NDStore *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return ND_BAD;
}
int nd_reorder(NDStore *arg0, const unsigned *arg1, size_t arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return ND_BAD;
}
int nd_record(const NDStore *arg0, unsigned arg1, NDRecord *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return ND_BAD;
}
