#include <stddef.h>
#ifndef LF_PROBE
#define LF_PROBE 1
#endif
#if LF_PROBE==1
#define EMPTY
#define SEQ(...) (0 __VA_OPT__(+) __VA_ARGS__)
int main(void){return SEQ()!=0 || SEQ(EMPTY)!=0 || SEQ(3)!=3 || SEQ(3+4)!=7;}
#elif LF_PROBE==2
[[nodiscard]] static int value(void){return 7;}
int main(void){return value()!=7;}
#elif LF_PROBE==3
/* Keyword spellings require no compatibility headers in C23. */
alignas(16) bool x;
static_assert(alignof(bool)<=16);int main(void){x=true;return !x;}
#elif LF_PROBE==4
_Alignas(16) _Bool x;
_Static_assert(_Alignof(_Bool)<=16,"selected alignment");int main(void){x=1;return !x;}
#elif LF_PROBE==5
#include <stdbool.h>
#include <stdalign.h>
#include <assert.h>
alignas(16) bool x;static_assert(alignof(bool)<=16,"selected alignment");int main(void){x=true;return !x;}
#elif LF_PROBE==6
#include <stdckdint.h>
int main(void){int value=0;return ckd_add(&value,1,2)||value!=3;}
#elif LF_PROBE==7
#include <stdbit.h>
int main(void){return stdc_bit_width(0u)!=0 || stdc_leading_zeros((unsigned char)0)!=8 || stdc_trailing_zeros((unsigned char)0)!=8;}
#elif LF_PROBE==8
/* Documented GNU statement expression; braces are not ISO C. */
#define ONCE(x) ({int lf_local=(x);lf_local+lf_local;})
int main(void){int n=0;int value=ONCE(++n);return n!=1 || value!=2;}
#elif LF_PROBE==9
static inline int twice(int n){return n+n;}
int main(void){int n=0;int value=twice(++n);return n!=1 || value!=2;}
#else
#error Unknown selection
#endif
