#include "migration.h"
#include <limits.h>
#ifndef LP_MUTANT
#define LP_MUTANT 0
#endif
int lp_sum(int a,int b,int*out){
 if(!out)return LP_INVALID;
 if(LP_MUTANT==1)*out=0;
 if((b>0&&a>INT_MAX-b)||(b<0&&a<INT_MIN-b))return LP_RANGE;
 *out=a+b;return LP_OK;
}
int lp_compare(const LpRecord*a,const LpRecord*b){
 if(LP_MUTANT==2)return (unsigned)a->key>(unsigned)b->key?1:(unsigned)a->key<(unsigned)b->key?-1:0;
 return (a->key>b->key)-(a->key<b->key);
}
int lp_adapter(const void*a,const void*b){return lp_compare(a,b);}
int lp_context_adapter(const void*a,const void*b,void*context){
 const LpOrder*order=context;int result=lp_adapter(a,b);return order->reverse&&LP_MUTANT!=3?-result:result;
}
int lp_sort(LpRecord*a,size_t n,const LpOrder*order){
 if((!a&&n)||!order||order->reverse>1)return LP_INVALID;
 if(n>16)return LP_RANGE;
 /* Copy the read-only order to make callback borrowing explicit. */
 LpOrder context=*order;
 for(size_t i=1;i<n;++i){LpRecord value=a[i];size_t j=i;
  while(j){int comparison=lp_context_adapter(&a[j-1],&value,&context);if(comparison<0||(comparison==0&&LP_MUTANT!=4))break;a[j]=a[j-1];--j;}
  a[j]=value;
 }
 return LP_OK;
}
