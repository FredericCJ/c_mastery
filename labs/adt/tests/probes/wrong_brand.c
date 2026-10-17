#include "adt.h"
int probe(ADPool *pool) { ADNodeIndex index={0}; uint32_t out; return ad_pool_get(pool,index,&out); }
