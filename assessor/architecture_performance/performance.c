#include "performance.h"
#include "bound.h"
static uint64_t gcd(uint64_t a,uint64_t b){while(b){uint64_t t=a%b;a=b;b=t;}return a;}
static struct pe_ratio reduce(struct pe_ratio a){uint64_t g=gcd(a.n,a.d);return (struct pe_ratio){a.n/g,a.d/g};}
static int mul(struct pe_ratio a,struct pe_ratio b,struct pe_ratio *r){uint64_t x=gcd(a.n,b.d),y=gcd(b.n,a.d);a.n/=x;b.d/=x;b.n/=y;a.d/=y;if((b.n&&a.n>UINT64_MAX/b.n)||a.d>UINT64_MAX/b.d)return 1;*r=reduce((struct pe_ratio){a.n*b.n,a.d*b.d});return 0;}
static int add(struct pe_ratio a,struct pe_ratio b,struct pe_ratio *r){uint64_t g=gcd(a.d,b.d),x=b.d/g,y=a.d/g;if((x&&a.n>UINT64_MAX/x)||(y&&b.n>UINT64_MAX/y)||a.d>UINT64_MAX/x)return 1;uint64_t l=a.n*x,h=b.n*y;if(l>UINT64_MAX-h)return 1;*r=reduce((struct pe_ratio){l+h,a.d*x});return 0;}
int pe_equation(uint64_t count,struct pe_ratio cpi,uint64_t hz,struct pe_ratio fraction,struct pe_ratio speed,uint64_t communication,struct pe_equation *out){
 if(!out||!count||!cpi.n||!cpi.d||!hz||!fraction.d||fraction.n>fraction.d||!speed.n||!speed.d)return 1;
 struct pe_equation r={0};struct pe_ratio cycles,remaining,fast,slow,floor;
 if(mul((struct pe_ratio){count,1},cpi,&cycles)||mul(cycles,(struct pe_ratio){1000000000,hz},&r.baseline_ns))return 1;
 if(mul(fraction,(struct pe_ratio){speed.d,speed.n},&fast)||add((struct pe_ratio){fraction.d-fraction.n,fraction.d},fast,&remaining))return 1;
 if(mul(r.baseline_ns,remaining,&r.new_ns))return 1;
#ifndef PE_MUTANT_COMM
 if(add(r.new_ns,(struct pe_ratio){communication,1},&r.new_ns))return 1;
#endif
 if(mul(r.baseline_ns,(struct pe_ratio){r.new_ns.d,r.new_ns.n},&r.speedup))return 1;
 if(mul(r.baseline_ns,(struct pe_ratio){fraction.d-fraction.n,fraction.d},&slow)||add(slow,(struct pe_ratio){communication,1},&floor))return 1;
 r.unbounded_limit=floor.n==0;
 if(!r.unbounded_limit&&mul(r.baseline_ns,(struct pe_ratio){floor.d,floor.n},&r.infinite_limit))return 1;
#ifdef PE_MUTANT_UNITS
 r.baseline_ns.n/=1000;
#endif
 *out=r;return 0;
}
int pe_stats(const uint64_t *a,size_t n,struct pe_stats *out){
 if(!a||!out||!n||n>128)return 1;
 uint64_t copy[128];for(size_t i=0;i<n;++i){uint64_t x=a[i];size_t j=i;while(j&&copy[j-1]>x){copy[j]=copy[j-1];--j;}copy[j]=x;}
 uint64_t lo=copy[(n-1)/2],hi=copy[n/2];struct pe_stats r={copy[0],copy[n-1],lo/2+hi/2+((lo%2+hi%2)/2),(unsigned)((lo%2+hi%2)%2)};
#ifdef PE_MUTANT_OUTLIER
 if(n>1)r.maximum=copy[n-2];
#endif
 *out=r;return 0;
}
int pe_c(const uint32_t *a,uint32_t n,uint32_t key,uint32_t *out){return algo_lower(a,n,key,out,0);}
int pe_asm(const uint32_t *a,uint32_t n,uint32_t key,uint32_t *out){
 uint32_t comparisons;
#ifdef PE_MUTANT_TRUST
 return ab_checked(a,n,n,key,1,out,&comparisons);
#else
 return ab_checked(a,n,n,key,0,out,&comparisons);
#endif
}
