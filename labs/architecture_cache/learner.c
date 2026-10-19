#include "cache.h"
enum ACHStatus ach_trace(const int64_t*a,size_t n,ACHConfig c,ACHTrace*out){(void)a;(void)n;(void)c;(void)out;return ACH_INVALID;}
enum ACHStatus ach_cost(ACHCostInput x,ACHCost*out){(void)x;(void)out;return ACH_INVALID;}
enum ACHStatus ach_overlap(const ACHMiss*a,size_t n,unsigned slots,ACHOverlap*out){(void)a;(void)n;(void)slots;(void)out;return ACH_INVALID;}
