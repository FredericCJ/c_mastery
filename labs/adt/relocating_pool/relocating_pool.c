#include "relocating_pool.h"
enum adp_status adp_ids_init(struct adp_ids *s,uint64_t n){(void)s;(void)n;return ADP_INVALID;}
enum adp_status adp_issue(struct adp_ids *s,struct adp_id *o){(void)s;(void)o;return ADP_INVALID;}
enum adp_status adp_init(struct adp_pool *p,struct adp_id i,struct adp_ops o,uint64_t g){(void)p;(void)i;(void)o;(void)g;return ADP_INVALID;}
enum adp_status adp_acquire(struct adp_pool *p,uint32_t v,struct adp_handle *o){(void)p;(void)v;(void)o;return ADP_INVALID;}
enum adp_status adp_get(struct adp_pool *p,struct adp_handle h,uint32_t *o){(void)p;(void)h;(void)o;return ADP_INVALID;}
enum adp_status adp_release(struct adp_pool *p,struct adp_handle h){(void)p;(void)h;return ADP_INVALID;}
enum adp_status adp_snapshot(struct adp_pool *p,struct adp_snapshot *o){(void)p;(void)o;return ADP_INVALID;}
enum adp_status adp_compact(struct adp_pool *p){(void)p;return ADP_INVALID;}
enum adp_status adp_move(struct adp_pool *d,struct adp_pool *s){(void)d;(void)s;return ADP_INVALID;}
enum adp_status adp_close(struct adp_pool *p){(void)p;return ADP_INVALID;}
enum adp_status adp_destroy(struct adp_pool *p){(void)p;return ADP_INVALID;}
