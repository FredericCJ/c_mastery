#include "traversal.h"
AtResult at_begin(const AcEpochOwner *o,unsigned l,AtCursor *c){(void)o;(void)l;(void)c;return AT_UNIMPLEMENTED;}
AtResult at_next(AtCursor *c,unsigned char *v){(void)c;(void)v;return AT_UNIMPLEMENTED;}
AtResult at_push(AtPush *h,const AcEpochOwner *o,unsigned l,AtVisitor v,void *c){(void)h;(void)o;(void)l;(void)v;(void)c;return AT_UNIMPLEMENTED;}
AtResult at_defer(AtDeferred *d,const AcEpochOwner *o,unsigned l,AtVisitor v,void *c){(void)d;(void)o;(void)l;(void)v;(void)c;return AT_UNIMPLEMENTED;}
AtResult at_pump(AtDeferred *d,unsigned b){(void)d;(void)b;return AT_UNIMPLEMENTED;}
AtResult at_cancel(AtDeferred *d){(void)d;return AT_UNIMPLEMENTED;}
AtResult at_recursive(const AtNode *n,size_t c,unsigned r,unsigned l,unsigned char *o,size_t cap,size_t *w){(void)n;(void)c;(void)r;(void)l;(void)o;(void)cap;(void)w;return AT_UNIMPLEMENTED;}
AtResult at_continuations(const AtNode *n,size_t c,unsigned r,unsigned l,unsigned char *o,size_t cap,size_t *w){(void)n;(void)c;(void)r;(void)l;(void)o;(void)cap;(void)w;return AT_UNIMPLEMENTED;}
