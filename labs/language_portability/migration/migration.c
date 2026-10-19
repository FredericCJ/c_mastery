#include "migration.h"
int lp_sum(int a,int b,int*c){(void)a;(void)b;(void)c;return LP_INVALID;}
int lp_compare(const LpRecord*a,const LpRecord*b){(void)a;(void)b;return 0;}
int lp_adapter(const void*a,const void*b){(void)a;(void)b;return 0;}
int lp_context_adapter(const void*a,const void*b,void*c){(void)a;(void)b;(void)c;return 0;}
int lp_sort(LpRecord*a,size_t b,const LpOrder*c){(void)a;(void)b;(void)c;return LP_INVALID;}
