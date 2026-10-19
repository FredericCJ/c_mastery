#ifndef BF_FIXED_POINT_H
#define BF_FIXED_POINT_H
#include "depth.h"
typedef struct {int32_t product_sum,rounded_raw,error_numerator;int8_t output;unsigned clipped;} BfSignal;
int bf_q7_even(const int8_t *,const int8_t *,size_t,int8_t *);
int bf_q11(const int16_t *,const int16_t *,size_t,int16_t *);
int bf_signal(const int8_t *,const int8_t *,size_t,BfSignal *);
typedef struct {int32_t raw_numerator,raw_denominator,rounded_raw,error_numerator;int8_t output;unsigned clipped;} BfSmall;
typedef struct {BfSmall first,second;int32_t error_numerator;unsigned bound_numerator,bounded;} BfCascade;
enum {BF_AWAY,BF_EVEN};
int bf_mul16(int8_t,int8_t,unsigned,BfSmall *);
int bf_mix16_32(int8_t,int8_t,unsigned,BfSmall *);
int bf_two16(int8_t,int8_t,int8_t,unsigned,BfCascade *);
#endif
