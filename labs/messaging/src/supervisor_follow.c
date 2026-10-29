#include "supervisor.h"
enum msg_status mfs_init(struct mfs_tree*t,size_t p,size_t c,uint64_t w,mfs_open_fn o,mfs_close_fn x,void*r,mfa_handler_fn h,void*a[4][4],mfa_dead_letter_fn d,void*z){(void)t;(void)p;(void)c;(void)w;(void)o;(void)x;(void)r;(void)h;(void)a;(void)d;(void)z;return MSG_FAILED;}
enum msg_status mfs_fault(struct mfs_tree*t,unsigned g,unsigned c,uint64_t n,enum msg_strategy s,unsigned f,struct mfs_report*r){(void)t;(void)g;(void)c;(void)n;(void)s;(void)f;(void)r;return MSG_FAILED;}
enum msg_status mfs_destroy(struct mfs_tree*t){(void)t;return MSG_FAILED;}
