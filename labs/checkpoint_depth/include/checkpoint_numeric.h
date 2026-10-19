#ifndef CHECKPOINT_NUMERIC_H
#define CHECKPOINT_NUMERIC_H
#include <stddef.h>
#include <stdint.h>
enum { CPN_BLOCK=256 };
typedef enum { CPN_OK, CPN_CAPACITY, CPN_INVALID, CPN_OVERLAP, CPN_UNSUPPORTED, CPN_FAILED } CPNStatus;
typedef struct { int32_t previous,older; } CPNExact;
typedef struct { double previous,older; } CPNDouble;
void cpn_exact_init(CPNExact *);
void cpn_double_init(CPNDouble *);
CPNStatus cpn_exact(CPNExact *,const int32_t *,size_t,int64_t *,size_t);
CPNStatus cpn_double(CPNDouble *,const double *,size_t,double *,size_t);
CPNStatus cpn_transaction(CPNDouble *,const double *,size_t,double *,size_t,size_t,int,size_t *);
CPNStatus cpn_round_even(int64_t,int32_t *);
CPNStatus cpn_iir_half(double *,const double *,size_t,double *,size_t);
#endif
