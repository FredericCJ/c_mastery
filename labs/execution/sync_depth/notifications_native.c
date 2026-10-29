#include "notifications_native.h"
int cdnn_init(struct cdn_native *p,struct cdn_handle h) { (void)p;(void)h;return CMX_INVALID; }
int cdnn_post(struct cdn_native *p,uint32_t n,struct cdnn_observation *o) { (void)p;(void)n;(void)o;return CMX_INVALID; }
int cdnn_take(struct cdn_native *p,uint32_t n,struct cdnn_observation *o) { (void)p;(void)n;(void)o;return CMX_INVALID; }
int cdnn_snapshot(struct cdn_native *p,struct cdnn_state *s) { (void)p;(void)s;return CMX_INVALID; }
int cdnn_destroy(struct cdn_native *p) { (void)p;return CMX_INVALID; }
