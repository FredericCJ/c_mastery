#include "sort.h"
int as_fallible(algo_pair*a,uint32_t n,uint32_t capacity,as_compare_fn compare,void*context,struct as_stats*out){
 if(!out || !compare || n>32 || n>capacity || (!a&&n))return 1;
 algo_pair staged[32];for(uint32_t i=0;i<n;++i)staged[i]=a[i];struct as_stats work={0};
 for(uint32_t i=1;i<n;++i){algo_pair value=staged[i];uint32_t j=i;
  while(j){int relation=0;++work.comparisons;int status=compare(context,staged[j-1].key,value.key,&relation);
   if(status || relation< -1 || relation>1){
#ifdef AS_MUTANT_FAILURE
    for(uint32_t k=0;k<n;++k)a[k]=staged[k];
#endif
    *out=work;return 2;
   }
   if(relation<=0)break;
   staged[j]=staged[j-1];++work.shifts;--j;
  }
  staged[j]=value;++work.placements;
 }
 for(uint32_t i=0;i<n;++i)a[i]=staged[i];
 *out=work;return 0;
}
