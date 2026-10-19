#include "interval.h"
int ni_ratio(int32_t n,int32_t d,unsigned bits,NiInterval *out) {
 (void)n;(void)d;(void)bits;(void)out;return NI_INVALID;
}
int ni_binary(unsigned op,NiInterval a,NiInterval b,NiInterval *out) {
 (void)op;(void)a;(void)b;(void)out;return NI_INVALID;
}
int ni_square(NiInterval a,NiInterval *out) {(void)a;(void)out;return NI_INVALID;}
int ni_rescale(NiInterval a,unsigned bits,NiInterval *out) {
 (void)a;(void)bits;(void)out;return NI_INVALID;
}
