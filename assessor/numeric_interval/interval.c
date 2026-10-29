#include "interval.h"
#include <limits.h>
#ifndef NI_MUTANT
#define NI_MUTANT 0
#endif
static int valid(struct ni_interval a) { return a.bits<=16 && a.lo<=a.hi; }
/* The admitted numerator is never INT64_MIN, denominator never zero. */
static int64_t down(int64_t n, int64_t d) {
 int64_t q=n/d,r=n%d;
 if(NI_MUTANT!=1 && r && ((r<0)!=(d<0))) --q;
 return q;
}
static int64_t up(int64_t n, int64_t d) {
 int64_t q=n/d,r=n%d;
 if(r && ((r<0)==(d<0))) ++q;
 return q;
}
static int publish(int64_t lo,int64_t hi,unsigned bits,struct ni_interval *out) {
 struct ni_interval result;
 if(lo<INT32_MIN || hi>INT32_MAX) return NI_RANGE;
 result.lo=(int32_t)lo; result.hi=(int32_t)hi; result.bits=bits;
 *out=result; return NI_OK;
}
int ni_ratio(int32_t n,int32_t d,unsigned bits,struct ni_interval *out) {
 int64_t scaled;
 if(!out || bits>16 || d<=0) return NI_INVALID;
 scaled=(int64_t)n*((int64_t)1<<bits);
 return publish(down(scaled,d),up(scaled,d),bits,out);
}
int ni_binary(unsigned op,struct ni_interval a,struct ni_interval b,struct ni_interval *out) {
 int64_t lo=0,hi=0,scale;
 if(!out || !valid(a) || !valid(b) || op>NI_INTERSECT) return NI_INVALID;
 if(a.bits!=b.bits) return NI_SCALE;
 scale=(int64_t)1<<a.bits;
 if(op==NI_ADD) { lo=(int64_t)a.lo+b.lo;hi=(int64_t)a.hi+b.hi; }
 else if(op==NI_SUB) { lo=(int64_t)a.lo-b.hi;hi=(int64_t)a.hi-b.lo; }
 else if(op==NI_MUL || op==NI_DIV) {
  int32_t av[2]={a.lo,a.hi},bv[2]={b.lo,b.hi};unsigned i,j;
  if(op==NI_DIV && b.lo<=0 && b.hi>=0) return NI_ZERO;
  for(i=0;i<2;++i) for(j=0;j<2;++j) {
   int64_t n=op==NI_MUL?(int64_t)av[i]*bv[j]:(int64_t)av[i]*scale;
   int64_t d=op==NI_MUL?scale:bv[j];
   int64_t l=down(n,d),h=up(n,d);
   if(!i&&!j) {lo=l;hi=h;}
   else {if(l<lo)lo=l;if(h>hi)hi=h;}
  }
 } else if(op==NI_HULL) {
  lo=a.lo<b.lo?a.lo:b.lo;hi=a.hi>b.hi?a.hi:b.hi;
 } else {
  lo=a.lo>b.lo?a.lo:b.lo;hi=a.hi<b.hi?a.hi:b.hi;
  if(lo>hi) return NI_EMPTY;
 }
 return publish(lo,hi,a.bits,out);
}
int ni_square(struct ni_interval a,struct ni_interval *out) {
 int64_t x,y,lo,hi,s;
 if(!out || !valid(a)) return NI_INVALID;
 x=(int64_t)a.lo*a.lo;y=(int64_t)a.hi*a.hi;s=(int64_t)1<<a.bits;
 lo=x<y?x:y;hi=x>y?x:y;
 if(NI_MUTANT!=2 && a.lo<=0 && a.hi>=0)lo=0;
 return publish(down(lo,s),up(hi,s),a.bits,out);
}
int ni_rescale(struct ni_interval a,unsigned bits,struct ni_interval *out) {
 int64_t lo,hi,scale;
 if(!out || !valid(a) || bits>16) return NI_INVALID;
 if(bits>=a.bits) {
  scale=(int64_t)1<<(bits-a.bits);lo=(int64_t)a.lo*scale;hi=(int64_t)a.hi*scale;
 } else {
  scale=(int64_t)1<<(a.bits-bits);lo=down(a.lo,scale);hi=up(a.hi,scale);
 }
 return publish(lo,hi,bits,out);
}
