#include "cache.h"
enum ach_status ach_trace(const int64_t*a,size_t n,struct ach_config c,struct ach_trace*out){(void)a;(void)n;(void)c;(void)out;return ACH_INVALID;}
enum ach_status ach_cost(struct ach_cost_input x,struct ach_cost*out){(void)x;(void)out;return ACH_INVALID;}
enum ach_status ach_overlap(const struct ach_miss*a,size_t n,unsigned slots,struct ach_overlap*out){(void)a;(void)n;(void)slots;(void)out;return ACH_INVALID;}
