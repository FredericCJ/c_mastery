#include "generic.h"
AsStatus av_text(const unsigned char *p,size_t n,AsValue *o){(void)p;(void)n;(void)o;return AS_UNIMPLEMENTED;}
AsStatus av_owned(const unsigned char *p,size_t n,AcAllocate a,AsValue *o){(void)p;(void)n;(void)a;(void)o;return AS_UNIMPLEMENTED;}
AsStatus av_clone(const AsValue *s,AsValue *o){(void)s;(void)o;return AS_UNIMPLEMENTED;}
AsStatus av_move(AsValue *s,AsValue *o,int f){(void)s;(void)o;(void)f;return AS_UNIMPLEMENTED;}
AsStatus av_drop(AsValue *v){(void)v;return AS_UNIMPLEMENTED;}
AsStatus av_valid(const AsValue *v){(void)v;return AS_UNIMPLEMENTED;}
AsValue av_parse(const unsigned char *p,size_t n){(void)p;(void)n;return (AsValue){.tag=AV_ERROR,.as.error={-99,0}};}
AsStatus av_validate_fields(const AvText *f,size_t n,AsValue r[4]){(void)f;(void)n;(void)r;return AS_UNIMPLEMENTED;}
AsStatus as_int_push(AsInts *b,int v){(void)b;(void)v;return AS_UNIMPLEMENTED;}
AsStatus as_erased_append(AsErased *b,const void *s,size_t n,void *t,size_t c,size_t a){(void)b;(void)s;(void)n;(void)t;(void)c;(void)a;return AS_UNIMPLEMENTED;}
AsStatus as_erased_clear(AsErased *b){(void)b;return AS_UNIMPLEMENTED;}
AsStatus as_value_copy_element(void *d,const void *s,void *c){(void)d;(void)s;(void)c;return AS_UNIMPLEMENTED;}
void as_value_drop_element(void *d,void *c){(void)d;(void)c;}
AsStatus as_int_copy_element(void *d,const void *s,void *c){(void)d;(void)s;(void)c;return AS_UNIMPLEMENTED;}
void as_int_drop_element(void *d,void *c){(void)d;(void)c;}
