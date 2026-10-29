#include "logger.h"
int ef_log(struct ef_log*l,const unsigned char*p,size_t n){if(!l||(!p&&n)||n>32)return -1;if(atomic_flag_test_and_set_explicit(&l->owned,memory_order_acquire))return 0;for(size_t i=0;i<n;++i)l->bytes[i]=p[i];l->length=n;atomic_flag_clear_explicit(&l->owned,memory_order_release);return 1;}
