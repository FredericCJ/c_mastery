#include "heap.h"
struct ah_stats ah_stats;
struct ah_trace ah_observed;
void ah_snapshot(const uint32_t*a,uint32_t n,uint32_t p){unsigned at=ah_observed.used++;if(at>=12||n>1024)return;ah_observed.position[at]=p;for(unsigned i=0;i<n;++i)ah_observed.state[at][i]=a[i];}
