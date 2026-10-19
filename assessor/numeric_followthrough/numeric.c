#include "numeric.h"
#include <float.h>
#include <string.h>
_Static_assert(FLT_RADIX==2 && DBL_MANT_DIG==53 && DBL_MAX_EXP==1024 && sizeof(double)==8,"selected binary64 profile");
#ifndef NF_MUTANT
#define NF_MUTANT 0
#endif
static double magnitude(double x){return x<0?-x:x;}
int nf_compare(NfPolicy policy,double want,double got,NfError *out){
 if(!out || (policy!=NF_TELEMETRY && policy!=NF_AUDIT) || !(want>=-1e100 && want<=1e100) || !(got>=-1e100 && got<=1e100))return -1;
 double scale=magnitude(want),error=magnitude(got-want);NfError next={error,error/(scale>1?scale:1),0};
 next.accepted=policy==NF_AUDIT?!memcmp(&want,&got,sizeof want):error<=0x1p-40+0x1p-40*scale;
#if NF_MUTANT==1
 next.accepted=error<=0x1p-40*scale;
#elif NF_MUTANT==2
 next.accepted=error<=0x1p-40+0x1p-40*scale;
#endif
 *out=next;return 0;
}
