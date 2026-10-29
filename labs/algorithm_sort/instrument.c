/* Supplied observer. Its own copies are instrumentation overhead, not kernel moves. */
#include "sort.h"
struct as_stats as_stats;
struct as_prefixes as_prefixes;
void as_prefix(const algo_pair*a,uint32_t n,uint32_t prefix){
 if(n>32 || prefix>n || as_prefixes.used>=32)return;
 uint32_t row=as_prefixes.used++;as_prefixes.length[row]=prefix;
 for(uint32_t i=0;i<n;++i)as_prefixes.state[row][i]=a[i];
 ++as_stats.prefixes;
}
