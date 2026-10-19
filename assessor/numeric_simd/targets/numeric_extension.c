#include "numeric_extension.h"
#include <limits.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif
int nd_q312(int16_t a,int16_t b,unsigned mode,int16_t*out){if(!out||mode>ND_ERROR)return 0;int32_t q=((int32_t)a*b)/4096;if(q<INT16_MIN||q>INT16_MAX){if(mode==ND_ERROR&&ND_MUTANT!=1)return 0;if(mode==ND_SATURATE)q=q<0?INT16_MIN:INT16_MAX;else{uint32_t bits=(uint32_t)q&65535u;q=bits<=32767?(int32_t)bits:(int32_t)bits-65536;}}*out=(int16_t)q;return 1;}
int nd_accumulate(int64_t*s,int64_t p){if(!s||(p>0&&*s>INT64_MAX-p)||(p<0&&*s<INT64_MIN-p))return 0;*s+=p;return 1;}
int nd_dot(const int16_t*a,const int16_t*b,size_t n,int64_t*out,unsigned lanes){if(!out||n>4096||(lanes!=1&&lanes!=4)||(n&&(!a||!b)))return 0;int64_t parts[4]={0};size_t stop=ND_MUTANT==2?n-n%lanes:n;for(size_t i=0;i<stop;i++)parts[i%lanes]+=(int32_t)a[i]*b[i];int64_t sum=0;for(unsigned j=0;j<lanes;j++)sum+=parts[j];*out=sum;return 1;}
static double pair(const double*a,size_t n){if(!n)return 0;if(n==1)return a[0];size_t m=n/2;return pair(a,m)+pair(a+m,n-m);}
int nd_pairwise(const double*a,size_t n,double*out){if(!out||n>257||(n&&!a))return 0;for(size_t i=0;i<n;i++)if(!(a[i]==a[i]&&a[i]>=-1e100&&a[i]<=1e100))return 0;*out=pair(a,n);return 1;}
int nd_matrix(const double*a,const double*b,size_t m,size_t n,size_t k,double*out,size_t cap,unsigned mode){if(m>64||n>64||k>64||mode>2||cap<m*n||(m&&n&&!out)||(m&&k&&!a)||(k&&n&&!b))return 0;for(size_t i=0;i<m*k;i++)if(!(a[i]==a[i]&&a[i]>=-7&&a[i]<=7))return 0;for(size_t i=0;i<k*n;i++)if(!(b[i]==b[i]&&b[i]>=-7&&b[i]<=7))return 0;double bt[4096];if(mode==1)for(size_t j=0;j<n;j++)for(size_t h=0;h<k;h++)bt[j*k+h]=b[h*n+j];for(size_t i=0;i<m*n;i++)out[i]=0;
 if(mode!=2){for(size_t i=0;i<m;i++)for(size_t j=0;j<n;j++)for(size_t h=0;h<k;h++)out[i*n+j]+=a[i*k+h]*(mode==1?bt[j*k+h]:b[h*n+j]);}
 else for(size_t ii=0;ii<m;ii+=3)for(size_t jj=0;jj<n;jj+=3)for(size_t hh=0;hh<k;hh+=3)for(size_t i=ii;i<m&&i<ii+3;i++)for(size_t j=jj;j<n&&j<jj+3;j++)for(size_t h=hh;h<k&&h<hh+3;h++)if(ND_MUTANT!=3||(i+1<m&&j+1<n))out[i*n+j]+=a[i*k+h]*b[h*n+j];return 1;}
