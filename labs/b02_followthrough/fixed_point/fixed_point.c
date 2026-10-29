#include "fixed_point.h"
int bf_q7_even(const int8_t*a,const int8_t*b,size_t n,int8_t*out){(void)a;(void)b;(void)n;(void)out;return B2_INVALID;}
int bf_q11(const int16_t*a,const int16_t*b,size_t n,int16_t*out){(void)a;(void)b;(void)n;(void)out;return B2_INVALID;}
int bf_signal(const int8_t*a,const int8_t*b,size_t n,struct bf_signal*out){(void)a;(void)b;(void)n;(void)out;return B2_INVALID;}
int bf_mul16(int8_t a,int8_t b,unsigned m,struct bf_small*out){(void)a;(void)b;(void)m;(void)out;return B2_INVALID;}
int bf_mix16_32(int8_t a,int8_t b,unsigned m,struct bf_small*out){(void)a;(void)b;(void)m;(void)out;return B2_INVALID;}
int bf_two16(int8_t a,int8_t b,int8_t c,unsigned m,struct bf_cascade*out){(void)a;(void)b;(void)c;(void)m;(void)out;return B2_INVALID;}
