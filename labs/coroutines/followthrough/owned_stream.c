#include "owned_stream.h"
int cf_item_drop(CFItem*a){(void)a;return CO_UNIMPLEMENTED;}
int cf_map_next(CFSource*a,CFMap b,void*c,CFItem*d){(void)a;(void)b;(void)c;(void)d;return CO_UNIMPLEMENTED;}
int cf_zip_next(CFSource*a,CFSource*b,CFMap c,void*d,CFItem*e,CFItem*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return CO_UNIMPLEMENTED;}
int cf_gen_create(COAllocator a,unsigned b,CFGenerator**c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int cf_gen_next(CFGenerator*a){(void)a;return CO_UNIMPLEMENTED;}
int cf_gen_borrow(CFGenerator*a,const unsigned char**b,size_t*c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int cf_gen_take(CFGenerator*a,CFItem*b){(void)a;(void)b;return CO_UNIMPLEMENTED;}
int cf_gen_clone(CFGenerator*a,CFItem*b){(void)a;(void)b;return CO_UNIMPLEMENTED;}
int cf_gen_destroy(CFGenerator**a){(void)a;return CO_UNIMPLEMENTED;}
