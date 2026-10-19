#include "formal.h"
#include <assert.h>
extern unsigned nondet_unsigned(void);
void __CPROVER_assume(_Bool);
int main(void){
 unsigned a[4],out[2]={99,123},expected=0,n=nondet_unsigned();
 __CPROVER_assume(n<=4);
#ifdef VACUOUS
 __CPROVER_assume(n<0u);
#endif
#ifdef BF_WITNESS
 assert(n != BF_WITNESS);
#endif
 for(unsigned i=0;i<4;++i){a[i]=nondet_unsigned();__CPROVER_assume(a[i]<=10);if(i<n)expected+=a[i];}
 assert(bf_sum(a,4,n,out)==BF_OK);assert(out[0]==expected);assert(out[1]==123);
 return 0;
}
