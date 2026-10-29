#include "generic.h"
enum as_status av_text(const unsigned char *p,size_t n,struct as_value *o){(void)p;(void)n;(void)o;return AS_UNIMPLEMENTED;}
enum as_status av_owned(const unsigned char *p,size_t n,struct ac_allocate a,struct as_value *o){(void)p;(void)n;(void)a;(void)o;return AS_UNIMPLEMENTED;}
enum as_status av_clone(const struct as_value *s,struct as_value *o){(void)s;(void)o;return AS_UNIMPLEMENTED;}
enum as_status av_move(struct as_value *s,struct as_value *o,int f){(void)s;(void)o;(void)f;return AS_UNIMPLEMENTED;}
enum as_status av_drop(struct as_value *v){(void)v;return AS_UNIMPLEMENTED;}
enum as_status av_valid(const struct as_value *v){(void)v;return AS_UNIMPLEMENTED;}
struct as_value av_parse(const unsigned char *p,size_t n){(void)p;(void)n;return (struct as_value){.tag=AV_ERROR,.as.error={-99,0}};}
enum as_status av_validate_fields(const struct av_text *f,size_t n,struct as_value r[4]){(void)f;(void)n;(void)r;return AS_UNIMPLEMENTED;}
enum as_status as_int_push(struct as_ints *b,int v){(void)b;(void)v;return AS_UNIMPLEMENTED;}
enum as_status as_erased_append(struct as_erased *b,const void *s,size_t n,void *t,size_t c,size_t a){(void)b;(void)s;(void)n;(void)t;(void)c;(void)a;return AS_UNIMPLEMENTED;}
enum as_status as_erased_clear(struct as_erased *b){(void)b;return AS_UNIMPLEMENTED;}
enum as_status as_value_copy_element(void *d,const void *s,void *c){(void)d;(void)s;(void)c;return AS_UNIMPLEMENTED;}
void as_value_drop_element(void *d,void *c){(void)d;(void)c;}
enum as_status as_int_copy_element(void *d,const void *s,void *c){(void)d;(void)s;(void)c;return AS_UNIMPLEMENTED;}
void as_int_drop_element(void *d,void *c){(void)d;(void)c;}
