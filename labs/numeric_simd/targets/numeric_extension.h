#ifndef CM_NUMERIC_EXTENSION_H
#define CM_NUMERIC_EXTENSION_H
#include <stdint.h>
#include <stddef.h>
enum {ND_WRAP,ND_SATURATE,ND_ERROR};
int nd_q312(int16_t,int16_t,unsigned,int16_t*);
int nd_dot(const int16_t*,const int16_t*,size_t,int64_t*,unsigned);
int nd_accumulate(int64_t*,int64_t);
/* Fixed midpoint tree, finite |value|<=1e100, n<=257; +0 for empty. */
int nd_pairwise(const double*,size_t,double*);
/* Dense row-major A[m*k], B[k*n], C[m*n], all disjoint live extents;
 * m,n,k <=64; zero dimensions permitted with empty output. Every input finite
 * and |value|<=7. mode0 scalar,1 transposed B,2 3x3 blocked. Same left-to-right
 * k reduction order, no contraction/reassociation. Output failure unchanged. */
int nd_matrix(const double*,const double*,size_t,size_t,size_t,double*,size_t,unsigned);
#endif
