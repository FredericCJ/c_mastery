#include "interval.h"
int ni_ratio(int32_t n,int32_t d,unsigned bits,struct ni_interval *out) {
 (void)n;(void)d;(void)bits;(void)out;return NI_INVALID;
}
int ni_binary(unsigned op,struct ni_interval a,struct ni_interval b,struct ni_interval *out) {
 (void)op;(void)a;(void)b;(void)out;return NI_INVALID;
}
int ni_square(struct ni_interval a,struct ni_interval *out) {(void)a;(void)out;return NI_INVALID;}
int ni_rescale(struct ni_interval a,unsigned bits,struct ni_interval *out) {
 (void)a;(void)bits;(void)out;return NI_INVALID;
}
