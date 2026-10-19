/* Synthetic ranges fit native int arithmetic independently of the checked guards.
 * Division is checked by a unique mathematical identity/sign/remainder predicate,
 * not a second copy of the quotient-adjustment implementation. */
#include "integer.h"
#include <stdio.h>
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"FI_ORACLE_FAIL line=%d a=%d b=%d op=%u\n",__LINE__,a,b,op);return 1;}}while(0)
static int magnitude(int n){return n<0?-n:n;}
int main(void) {
 const FiRange ranges[]={{-128,127},{-127,127},{-12,3}};
 unsigned long arithmetic=0,division=0,saturation=0;
 for(unsigned k=0;k<3;++k) {
  FiRange range=ranges[k];
  for(int a=range.low;a<=range.high;++a)for(int b=range.low;b<=range.high;++b) {
   for(unsigned op=0;op<3;++op) {
    /* Max magnitude16384, representable even by minimum ISO int range. */
    int exact=op==FI_ADD?a+b:op==FI_SUBTRACT?a-b:a*b;
    int admitted=exact>=range.low&&exact<=range.high,out=917;
    REQUIRE(fi_binary(range,(FiOperation)op,a,b,&out)==(admitted?FI_OK:FI_RANGE));
    REQUIRE(out==(admitted?exact:917));++arithmetic;
    unsigned clipped=99;out=917;
    REQUIRE(fi_saturate(range,(FiOperation)op,a,b,&out,&clipped)==FI_OK);
    REQUIRE(out==(exact<range.low?range.low:exact>range.high?range.high:exact));
    REQUIRE(clipped==(unsigned)!admitted);++saturation;
   }
   /* On the asymmetric small range, some otherwise valid quotients/remainders
    * can lie outside the selected range. A search gives independent existence. */
   for(unsigned op=0;op<3;++op) {
    int wantq=917,wantr=919,found=0;
    if(b)for(int candidate=range.low;candidate<=range.high;++candidate) {
     int residual=a-candidate*b;
     if(residual<range.low||residual>range.high||magnitude(residual)>=magnitude(b))continue;
     int sign=op==FI_TRUNCATE?(residual==0 || (residual<0)==(a<0)):
              op==FI_FLOOR?(residual==0 || (residual<0)==(b<0)):residual>=0;
     if(sign){REQUIRE(!found);found=1;wantq=candidate;wantr=residual;}
    }
    int q=917,r=919;
    /* Baseline rejects unrepresentable truncating quotient before either / or %,
     * even if another application's rounding convention could otherwise differ. */
    REQUIRE(fi_divide(range,(FiDivision)op,a,b,&q,&r)==(found?FI_OK:FI_RANGE));
    REQUIRE(q==wantq&&r==wantr);++division;
   }
  }
 }
 printf("FI_ORACLE_PASS arithmetic=%lu saturation=%lu division=%lu\n",arithmetic,saturation,division);return 0;
}
