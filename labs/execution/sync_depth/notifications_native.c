#include "notifications_native.h"
int cdnn_init(CDNNative *p,CDNHandle h) { (void)p;(void)h;return CMX_INVALID; }
int cdnn_post(CDNNative *p,uint32_t n,CDNNObservation *o) { (void)p;(void)n;(void)o;return CMX_INVALID; }
int cdnn_take(CDNNative *p,uint32_t n,CDNNObservation *o) { (void)p;(void)n;(void)o;return CMX_INVALID; }
int cdnn_snapshot(CDNNative *p,CDNNState *s) { (void)p;(void)s;return CMX_INVALID; }
int cdnn_destroy(CDNNative *p) { (void)p;return CMX_INVALID; }
