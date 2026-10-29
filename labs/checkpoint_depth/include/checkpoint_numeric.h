#ifndef CHECKPOINT_NUMERIC_H
#define CHECKPOINT_NUMERIC_H
#include <stddef.h>
#include <stdint.h>
enum { CPN_BLOCK=256 };
enum cpn_status { CPN_OK, CPN_CAPACITY, CPN_INVALID, CPN_OVERLAP, CPN_UNSUPPORTED, CPN_FAILED };
struct cpn_exact { int32_t previous,older; };
struct cpn_double { double previous,older; };
void cpn_exact_init(struct cpn_exact *);
void cpn_double_init(struct cpn_double *);
enum cpn_status cpn_exact(struct cpn_exact *,const int32_t *,size_t,int64_t *,size_t);
enum cpn_status cpn_double(struct cpn_double *,const double *,size_t,double *,size_t);
enum cpn_status cpn_transaction(struct cpn_double *,const double *,size_t,double *,size_t,size_t,int,size_t *);
enum cpn_status cpn_round_even(int64_t,int32_t *);
enum cpn_status cpn_iir_half(double *,const double *,size_t,double *,size_t);
#endif
