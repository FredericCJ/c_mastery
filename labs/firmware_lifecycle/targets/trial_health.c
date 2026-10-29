#include "trial_health.h"
int th_health_accept(unsigned c,uint32_t g,uint32_t e,uint32_t r){(void)c;(void)g;(void)e;(void)r;return 0;}
enum fh_result th_commit_begin(struct fm_recycle*r,struct fm_io io,const struct bs_selection*o,const struct bs_selection*t,const struct fw_peer*p,uint32_t g){(void)r;(void)io;(void)o;(void)t;(void)p;(void)g;return FH_REJECT;}
