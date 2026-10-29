#include "migration.h"
int lp_sum(int a,int b,int*c){(void)a;(void)b;(void)c;return LP_INVALID;}
int lp_compare(const struct lp_record*a,const struct lp_record*b){(void)a;(void)b;return 0;}
int lp_adapter(const void*a,const void*b){(void)a;(void)b;return 0;}
int lp_context_adapter(const void*a,const void*b,void*c){(void)a;(void)b;(void)c;return 0;}
int lp_sort(struct lp_record*a,size_t b,const struct lp_order*c){(void)a;(void)b;(void)c;return LP_INVALID;}
