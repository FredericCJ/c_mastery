#include "interval.h"
#include <limits.h>
#include <stdio.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"NI_ASSERT:%d:%s\n",__LINE__,#x);return 1;}} while(0)
static int same(NiInterval a,NiInterval b){return a.lo==b.lo&&a.hi==b.hi&&a.bits==b.bits;}
int main(void) {
 NiInterval a={-1,1,0},b={3,3,0},out={77,88,9},old=out;
 CHECK(ni_ratio(-1,3,8,&out)==NI_OK && out.lo==-86 && out.hi==-85);
 CHECK(ni_ratio(1,3,8,&out)==NI_OK && out.lo==85 && out.hi==86);
 CHECK(ni_square(a,&out)==NI_OK && out.lo==0 && out.hi==1);
 CHECK(ni_binary(NI_MUL,a,a,&out)==NI_OK && out.lo==-1 && out.hi==1);
 CHECK(ni_binary(NI_SUB,a,a,&out)==NI_OK && out.lo==-2 && out.hi==2);
 CHECK(ni_binary(NI_DIV,a,b,&out)==NI_OK && out.lo==-1 && out.hi==1);
 out=old;CHECK(ni_binary(NI_DIV,b,a,&out)==NI_ZERO && same(old,out));
 a=(NiInterval){INT32_MIN,INT32_MAX,16};out=old;
 CHECK(ni_square(a,&out)==NI_RANGE && same(old,out));
 CHECK(ni_binary(NI_ADD,a,a,&out)==NI_RANGE && same(old,out));
 a=(NiInterval){-257,257,16};CHECK(ni_rescale(a,8,&out)==NI_OK && out.lo==-2 && out.hi==2);
 a=(NiInterval){-256,256,16};CHECK(ni_rescale(a,8,&out)==NI_OK && out.lo==-1 && out.hi==1);
 CHECK(ni_rescale(out,16,&out)==NI_OK && same(a,out));
 a=(NiInterval){1,2,0};b=(NiInterval){2,3,0};
 CHECK(ni_binary(NI_INTERSECT,a,b,&out)==NI_OK && out.lo==2 && out.hi==2);
 b.lo=3;out=old;CHECK(ni_binary(NI_INTERSECT,a,b,&out)==NI_EMPTY && same(out,old));
 CHECK(ni_binary(NI_HULL,a,b,&out)==NI_OK && out.lo==1 && out.hi==3);
 out=old;b.bits=1;CHECK(ni_binary(NI_ADD,a,b,&out)==NI_SCALE && same(out,old));
 a.lo=4;CHECK(ni_binary(NI_ADD,a,b,&out)==NI_INVALID && same(out,old));
 CHECK(ni_ratio(INT32_MIN,1,16,&out)==NI_RANGE && same(out,old));
 CHECK(ni_ratio(1,0,0,&out)==NI_INVALID && same(out,old));
 CHECK(ni_ratio(0,1,17,&out)==NI_INVALID && same(out,old));
 CHECK(ni_ratio(0,1,0,0)==NI_INVALID);
 puts("PASS interval boundaries, correlation and precision transfer");return 0;
}
