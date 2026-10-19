#include "performance.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
static void *owned[3];
#define CHECK(c) do{if(!(c)){printf("BP_CHECK:%d:%s\n",__LINE__,#c);for(unsigned owner=0;owner<3;++owner)free(owned[owner]);return 1;}}while(0)
int main(void){
 unsigned cases=0;
 for(size_t n=0;n<=9;++n)for(size_t off=0;off<4;++off){
  /* Each allocation ends exactly at the admitted extent: sanitizers catch vector overreads. */
  size_t total=n+off+1;uint32_t *aa=malloc(total*sizeof *aa),*bb=malloc(total*sizeof *bb),*oo=malloc(total*sizeof *oo);
  owned[0]=aa;owned[1]=bb;owned[2]=oo;
  CHECK(aa&&bb&&oo);uint32_t *a=aa+off+1,*b=bb+off+1,*out=oo+off+1;out[-1]=123;
  for(size_t i=0;i<n;++i){a[i]=UINT32_MAX-(uint32_t)i;b[i]=(uint32_t)(2*i+2);out[i]=77;}
  for(int path=0;path<4;++path){bf_dispatch(a,b,out,n,path?8:0,1,path==3);for(size_t i=0;i<n;++i)CHECK(out[i]==(uint32_t)(i+1));CHECK(out[-1]==123);++cases;}
  bf_add_disjoint(a,b,out,n);for(size_t i=0;i<n;++i)CHECK(out[i]==(uint32_t)(i+1));
  for(unsigned lanes=1;lanes<=8;lanes*=2){for(size_t i=0;i<n;++i)out[i]=77;bf_add_unroll(a,b,out,n,lanes);for(size_t i=0;i<n;++i)CHECK(out[i]==(uint32_t)(i+1));}
  bf_add_inplace(a,b,n);for(size_t i=0;i<n;++i)CHECK(a[i]==(uint32_t)(i+1));
  free(aa);free(bb);free(oo);owned[0]=owned[1]=owned[2]=NULL;
 }
 CHECK(bf_select(0,1,0)==0&&bf_select(1,1,0)==1&&bf_select(8,1,0)==1&&bf_select(1,0,0)==0&&bf_select(8,0,0)==0&&bf_select(8,1,1)==0);
 uint32_t a[9]={0,1,2,3,4,5,6,UINT32_MAX,9},b[9]={1,1,1,1,1,1,1,1,1},o[9];bf_sse2(a,b,o,9);
 uint32_t expect[9]={1,2,3,4,5,6,7,0,10};for(size_t i=0;i<9;++i)CHECK(o[i]==expect[i]);
 uint32_t x[5]={0,UINT32_MAX,4,6,8},y[5]={1,1,5,4,2};bf_add(x,y,o,5);uint32_t e[5]={1,0,9,10,10};for(size_t i=0;i<5;++i)CHECK(o[i]==e[i]);
 int values[6]={INT_MIN,-2,0,3,5,INT_MAX},clamped[6]={0,0,0,3,4,4};for(size_t i=0;i<6;++i)CHECK(bf_clamp_branch(values[i])==clamped[i]&&bf_clamp_select(values[i])==clamped[i]);
 float f[3]={1.0e20f,-1.0e20f,3.0f};CHECK(bf_reduce(f,3)==3.0f);float nanv[1]={NAN};CHECK(isnan(bf_reduce(nanv,1)));float zero[1]={-0.0f};CHECK(bf_reduce(zero,1)==0.0f);
 struct bf_record r[3]={{10,100,"a"},{20,200,"b"},{30,300,"c"}};uint32_t id[3],v[3];size_t count=3;bf_to_soa(r,count,id,v);CHECK(bf_delete_soa(id,v,&count,20)==1&&count==2&&id[1]==30&&v[1]==300);CHECK(bf_delete_soa(id,v,&count,99)==0&&count==2);
 uint32_t matrix[3*7],transpose[5*4];for(size_t i=0;i<21;++i)matrix[i]=123;for(size_t i=0;i<20;++i)transpose[i]=456;
 for(size_t rr=0;rr<3;++rr)for(size_t col=0;col<5;++col)matrix[rr*7+col]=(uint32_t)(rr*10+col);
 bf_blocked(matrix,3,5,7,transpose,4,2);for(size_t col=0;col<5;++col){for(size_t rr=0;rr<3;++rr)CHECK(transpose[col*4+rr]==rr*10+col);CHECK(transpose[col*4+3]==456);}
 printf("BP_PASS:%u dispatch-alignment-tail cases; wrap; extrema; strict float; identity\n",cases);return 0;
}
