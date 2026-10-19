#include <stddef.h>
#ifndef DX_WEAK
#define DX_WEAK 0
#endif
/* The bound is an explicit entry premise, not inferred from one dynamic run. */
/*@ requires 0 <= n <= 4; */
int dx_static(unsigned n){int a[4]={1,2,3,4};if(DX_WEAK || n<4)return a[n];return 0;}
int main(int argc,char **argv){(void)argv;return dx_static(argc>1?4u:3u)==4?0:1;}
