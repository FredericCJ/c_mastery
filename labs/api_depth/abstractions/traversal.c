#include "traversal.h"
enum at_result at_begin(const struct ac_epoch_owner *o,unsigned l,struct at_cursor *c){(void)o;(void)l;(void)c;return AT_UNIMPLEMENTED;}
enum at_result at_next(struct at_cursor *c,unsigned char *v){(void)c;(void)v;return AT_UNIMPLEMENTED;}
enum at_result at_push(struct at_push *h,const struct ac_epoch_owner *o,unsigned l,at_visitor_fn v,void *c){(void)h;(void)o;(void)l;(void)v;(void)c;return AT_UNIMPLEMENTED;}
enum at_result at_defer(struct at_deferred *d,const struct ac_epoch_owner *o,unsigned l,at_visitor_fn v,void *c){(void)d;(void)o;(void)l;(void)v;(void)c;return AT_UNIMPLEMENTED;}
enum at_result at_pump(struct at_deferred *d,unsigned b){(void)d;(void)b;return AT_UNIMPLEMENTED;}
enum at_result at_cancel(struct at_deferred *d){(void)d;return AT_UNIMPLEMENTED;}
enum at_result at_recursive(const struct at_node *n,size_t c,unsigned r,unsigned l,unsigned char *o,size_t cap,size_t *w){(void)n;(void)c;(void)r;(void)l;(void)o;(void)cap;(void)w;return AT_UNIMPLEMENTED;}
enum at_result at_continuations(const struct at_node *n,size_t c,unsigned r,unsigned l,unsigned char *o,size_t cap,size_t *w){(void)n;(void)c;(void)r;(void)l;(void)o;(void)cap;(void)w;return AT_UNIMPLEMENTED;}
