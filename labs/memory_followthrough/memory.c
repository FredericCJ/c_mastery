/* Learner work: implement the declared contracts. */
#include "memory.h"
MwStatus mw_fill(unsigned char *p,size_t backing,MwSpan s,ptrdiff_t index,size_t n,unsigned char value){(void)p;(void)backing;(void)s;(void)index;(void)n;(void)value;return MW_UNIMPLEMENTED;}
MwStatus mw_scan(const unsigned char *p,size_t backing,MwSpan s,size_t *out){(void)p;(void)backing;(void)s;(void)out;return MW_UNIMPLEMENTED;}
MwStatus mw_string(unsigned char *p,size_t backing,MwSpan dst,MwSpan src,size_t n,size_t *out){(void)p;(void)backing;(void)dst;(void)src;(void)n;(void)out;return MW_UNIMPLEMENTED;}
MwStatus mw_move(unsigned char *p,size_t backing,MwSpan dst,MwSpan src,size_t n,size_t *out){(void)p;(void)backing;(void)dst;(void)src;(void)n;(void)out;return MW_UNIMPLEMENTED;}
MwStatus mw_size(size_t count,size_t stride,size_t header,size_t terminator,size_t alignment,size_t limit,size_t *out){(void)count;(void)stride;(void)header;(void)terminator;(void)alignment;(void)limit;(void)out;return MW_UNIMPLEMENTED;}
MwStatus mw_signed_size(intmax_t count,size_t stride,size_t header,size_t *out){(void)count;(void)stride;(void)header;(void)out;return MW_UNIMPLEMENTED;}
MwStatus mw_layout(size_t region,size_t alignment,size_t data,size_t bss,size_t retained,size_t heap,size_t guard,size_t stack,MwLayout *out){(void)region;(void)alignment;(void)data;(void)bss;(void)retained;(void)heap;(void)guard;(void)stack;(void)out;return MW_UNIMPLEMENTED;}
MwStatus mw_startup(unsigned char *dst,size_t n,const unsigned char *load,size_t ln,MwInterval data,MwInterval bss,MwInterval retained){(void)dst;(void)n;(void)load;(void)ln;(void)data;(void)bss;(void)retained;return MW_UNIMPLEMENTED;}
MwStatus mw_owner(MwOwner *o,MwAllocator a,size_t n){(void)o;(void)a;(void)n;return MW_UNIMPLEMENTED;}
MwStatus mw_resize(MwOwner *o,size_t n,unsigned copying){(void)o;(void)n;(void)copying;return MW_UNIMPLEMENTED;}
MwStatus mw_read(const MwOwner *o,MwBorrow b,unsigned char *out){(void)o;(void)b;(void)out;return MW_UNIMPLEMENTED;}
MwStatus mw_drop(MwOwner *o,uint32_t generation){(void)o;(void)generation;return MW_UNIMPLEMENTED;}
MwStatus mw_record(MwRecord *out,MwAllocator a,const unsigned char *src,size_t n,unsigned mode,unsigned version,unsigned optional,unsigned fail){(void)out;(void)a;(void)src;(void)n;(void)mode;(void)version;(void)optional;(void)fail;return MW_UNIMPLEMENTED;}
MwStatus mw_serialize(const MwRecord *r,unsigned char *out,size_t capacity,size_t *length){(void)r;(void)out;(void)capacity;(void)length;return MW_UNIMPLEMENTED;}
void mw_record_drop(MwRecord *r){(void)r;}
MwStatus mw_tree(unsigned depth,unsigned breadth,unsigned *work,size_t capacity,unsigned *out,size_t *peak){(void)depth;(void)breadth;(void)work;(void)capacity;(void)out;(void)peak;return MW_UNIMPLEMENTED;}
void mw_async_init(MwAsync *a){(void)a;}
MwStatus mw_submit(MwAsync *a,uint32_t request,MwTicket *out){(void)a;(void)request;(void)out;return MW_UNIMPLEMENTED;}
MwStatus mw_submit_return(MwAsync *a,MwTicket t){(void)a;(void)t;return MW_UNIMPLEMENTED;}
MwStatus mw_cancel(MwAsync *a,MwTicket t){(void)a;(void)t;return MW_UNIMPLEMENTED;}
MwStatus mw_terminal(MwAsync *a,MwTicket t){(void)a;(void)t;return MW_UNIMPLEMENTED;}
MwStatus mw_callback_begin(MwAsync *a,MwTicket t,unsigned scratch){(void)a;(void)t;(void)scratch;return MW_UNIMPLEMENTED;}
MwStatus mw_callback_end(MwAsync *a,MwTicket t){(void)a;(void)t;return MW_UNIMPLEMENTED;}
MwStatus mw_reclaim(MwAsync *a,MwTicket t){(void)a;(void)t;return MW_UNIMPLEMENTED;}
