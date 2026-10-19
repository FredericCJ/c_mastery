#include "rational.h"
#ifndef EFR_MUTATION
#define EFR_MUTATION 0
#endif
static uint64_t gcd64(uint64_t a,uint64_t b){while(b){uint64_t r=a%b;a=b;b=r;}return a;}
uint32_t efr_gcd(uint32_t a,uint32_t b,unsigned *steps){unsigned n=0;while(b){uint32_t r=a%b;a=b;b=r;++n;}if(steps)*steps=n;return a;}
static int valid(er_fraction a){er_fraction canonical;return er_refine(a.numerator,a.denominator,&canonical)==ER_OK&&canonical.numerator==a.numerator&&canonical.denominator==a.denominator;}
er_code efr_consume(er_fraction a,uint32_t *out){
 if(!out||!a.denominator||(EFR_MUTATION!=1&&!valid(a)))return ER_INVALID;
 *out=a.numerator/a.denominator;return ER_OK;
}
er_code efr_add_naive(er_fraction a,er_fraction b,er_fraction *out){
 if(!out||!valid(a)||!valid(b))return ER_INVALID;
 uint64_t x=(uint64_t)a.numerator*b.denominator,y=(uint64_t)b.numerator*a.denominator,d=(uint64_t)a.denominator*b.denominator;
 if(x>UINT32_MAX||y>UINT32_MAX||x+y>UINT32_MAX||d>UINT32_MAX)return ER_RANGE;
 er_fraction value;if(er_refine((uint32_t)(x+y),(uint32_t)d,&value)!=ER_OK)return ER_INVALID;
 *out=value;return ER_OK;
}
er_code efr_add(er_fraction a,er_fraction b,er_fraction *out){
 if(!out||!valid(a)||!valid(b))return ER_INVALID;
 uint64_t g=efr_gcd(a.denominator,b.denominator,NULL),left=a.denominator/g,right=b.denominator/g;
 uint64_t x=(uint64_t)a.numerator*right,y=(uint64_t)b.numerator*left;
 if(EFR_MUTATION==2)*out=(er_fraction){0,1};
 if(y>UINT64_MAX-x)return ER_RANGE;
 uint64_t t=x+y,h=gcd64(t,g),n=t/h,d=left*(b.denominator/h);
 if(EFR_MUTATION==3)d=(uint64_t)a.denominator*b.denominator;
 if(EFR_MUTATION==4){n=t;d=left*b.denominator;}
 if(n>UINT32_MAX||d>UINT32_MAX)return ER_RANGE;
 *out=(er_fraction){(uint32_t)n,(uint32_t)d};return ER_OK;
}
static uint32_t magnitude(int32_t n){return n<0?UINT32_C(0)-(uint32_t)n:(uint32_t)n;}
static int signed_valid(EfrSigned a){return a.denominator!=0&&efr_gcd(magnitude(a.numerator),a.denominator,NULL)==1;}
static er_code publish(uint64_t n,uint64_t d,int negative,EfrSigned *out){
 if(d==0||d>UINT32_MAX||n>(negative?UINT64_C(2147483648):INT32_MAX))return ER_RANGE;
 int32_t value;
 if(negative&&n==UINT64_C(2147483648))value=INT32_MIN;
 else value=negative?-(int32_t)n:(int32_t)n;
 *out=(EfrSigned){value,(uint32_t)d};return ER_OK;
}
er_code efr_signed_make(int32_t n,uint32_t d,EfrSigned *out){
 if(!out||!d)return ER_INVALID;
 if(EFR_MUTATION==5&&n==INT32_MIN)return ER_RANGE;
 uint32_t a=magnitude(n),g=efr_gcd(a,d,NULL);return publish(a/g,d/g,n<0,out);
}
er_code efr_signed_add(EfrSigned a,EfrSigned b,EfrSigned *out){
 if(!out||!signed_valid(a)||!signed_valid(b))return ER_INVALID;
 uint64_t g=efr_gcd(a.denominator,b.denominator,NULL),left=a.denominator/g,right=b.denominator/g;
 uint64_t x=(uint64_t)magnitude(a.numerator)*right,y=(uint64_t)magnitude(b.numerator)*left,t;int negative;
 if((a.numerator<0)==(b.numerator<0)||EFR_MUTATION==6){t=x+y;negative=a.numerator<0;}
 else if(x>=y){t=x-y;negative=a.numerator<0;}
 else {t=y-x;negative=b.numerator<0;}
 uint64_t h=gcd64(t,g);return publish(t/h,left*(b.denominator/h),negative,out);
}
er_code efr_signed_consume(EfrSigned a,int32_t *out){
 if(!out||!signed_valid(a))return ER_INVALID;
 uint32_t q=magnitude(a.numerator)/a.denominator;
 if(a.numerator<0&&q==UINT32_C(2147483648))*out=INT32_MIN;
 else *out=a.numerator<0?-(int32_t)q:(int32_t)q;
 return ER_OK;
}
