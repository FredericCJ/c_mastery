#include "workload.h"
void sfw_init(SfWork *s){(void)s;}
int sfw_offer(SfWork *s,unsigned n){(void)s;(void)n;return AF_UNIMPLEMENTED;}
int sfw_work(SfWork *s){(void)s;return AF_UNIMPLEMENTED;}
int sfw_quiescent(SfWork *s,unsigned p,unsigned d){(void)s;(void)p;(void)d;return AF_UNIMPLEMENTED;}
int sfw_check(const SfWork *s){(void)s;return AF_UNIMPLEMENTED;}
int sfw_run(SfWork *s,unsigned n){(void)s;(void)n;return AF_UNIMPLEMENTED;}
