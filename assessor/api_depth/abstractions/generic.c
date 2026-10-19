#include "generic.h"
#include <string.h>
#include <limits.h>
_Static_assert(INT_MIN<=-32768L&&INT_MAX>=32767,"selected signed interval must fit int");
AsStatus av_valid(const AsValue *v) {
 if(!v)return AS_INVALID;
 switch(v->tag) {
 case AV_INT:return AS_OK;
 case AV_TEXT:return v->as.text.length<=8&&(!v->as.text.length||v->as.text.data)?AS_OK:AS_INVALID;
 case AV_ERROR:return v->as.error.code<0?AS_OK:AS_INVALID;
 case AV_OWNED:return v->as.owned.length<=8&&v->as.owned.data&&v->as.owned.allocator.release&&v->as.owned.allocator.allocate?AS_OK:AS_INVALID;
 }
 return AS_INVALID;
}
AsStatus av_text(const unsigned char *p,size_t n,AsValue *out) {
 if(!out||n>8||(n&&!p))return AS_INVALID;
 *out=(AsValue){.tag=AV_TEXT,.as.text={p,n}};return AS_OK;
}
AsStatus av_owned(const unsigned char *p,size_t n,AcAllocate a,AsValue *out) {
 if(!out||n>8||(n&&!p)||!a.allocate||!a.release)return AS_INVALID;
 unsigned char *copy=a.allocate(a.context,n?n:1,1);if(!copy)return AS_FAILED;
 for(size_t i=0;i<n;++i)copy[i]=p[i];
 *out=(AsValue){.tag=AV_OWNED,.as.owned={copy,n,a}};return AS_OK;
}
AsStatus av_clone(const AsValue *source,AsValue *out) {
 if(!out||source==out||av_valid(source)!=AS_OK)return AS_INVALID;
 if(source->tag==AV_OWNED)return av_owned(source->as.owned.data,source->as.owned.length,source->as.owned.allocator,out);
 *out=*source;return AS_OK;
}
AsStatus av_move(AsValue *source,AsValue *out,int fail) {
 if(!out||source==out||av_valid(source)!=AS_OK)return AS_INVALID;
 if(fail)return AS_FAILED;
 *out=*source;*source=(AsValue){.tag=AV_INT,.as.integer=0};return AS_OK;
}
AsStatus av_drop(AsValue *value) {
 if(av_valid(value)!=AS_OK)return AS_INVALID;
 if(value->tag==AV_OWNED){AvOwned owned=value->as.owned;
  *value=(AsValue){.tag=AV_INT,.as.integer=0};owned.allocator.release(owned.allocator.context,owned.data);
 }else *value=(AsValue){.tag=AV_INT,.as.integer=0};
 return AS_OK;
}
static AsValue error(size_t offset,int code){return (AsValue){.tag=AV_ERROR,.as.error={code,offset}};}
AsValue av_parse(const unsigned char *p,size_t n) {
 if(!n)return error(0,-1);
 if(!p||n>8)return error(0,-4);
 size_t i=0;int negative=p[0]=='-';unsigned value=0;
 if(p[0]=='-'||p[0]=='+')++i;
 if(i==n)return error(i,-1);
 unsigned limit=negative?32768u:32767u;
 for(;i<n;++i) {
  if(p[i]<'0'||p[i]>'9')return error(i,-2);
  unsigned digit=p[i]-'0';
#ifndef AS_MUTANT_PARSE_RANGE
  if(value>limit/10||(value==limit/10&&digit>limit%10))return error(i,-3);
#else
  (void)limit;
#endif
  value=value*10+digit;
 }
 /* value<=32768. Avoid negating32768 in an int16 implementation. */
 int result=negative?-(int)(value?value-1:0)-(value?1:0):(int)value;
 return (AsValue){.tag=AV_INT,.as.integer=result};
}
AsStatus av_validate_fields(const AvText *fields,size_t count,AsValue out[4]) {
 if(!out||count>4||(count&&!fields))return AS_INVALID;
 for(size_t i=0;i<count;++i)out[i]=av_parse(fields[i].data,fields[i].length);
 return AS_OK;
}
AsStatus as_int_push(AsInts *b,int v){if(!b||b->count>4)return AS_INVALID;if(b->count==4)return AS_FULL;b->values[b->count++]=v;return AS_OK;}
static int descriptor(const AsErased *b) {
 return b&&b->data&&b->count<=b->capacity&&b->capacity<=4&&b->elements.size&&
 b->elements.size<=SIZE_MAX/4&&b->elements.alignment&&
 b->elements.size%b->elements.alignment==0&&b->elements.copy&&b->elements.drop;
}
AsStatus as_erased_append(AsErased *b,const void *source,size_t count,void *stage,size_t stage_capacity,size_t alignment) {
 if(!descriptor(b)||(count&&(!source||!stage))||stage_capacity<count||
    alignment<b->elements.alignment||alignment%b->elements.alignment)return AS_INVALID;
 if(count>b->capacity-b->count)return AS_FULL;
 size_t width=b->elements.size,done=0;
 for(;done<count;++done) {
  AsStatus result=b->elements.copy((unsigned char *)stage+done*width,(const unsigned char *)source+done*width,b->elements.context);
  if(result!=AS_OK) {
#ifndef AS_MUTANT_STAGE_LEAK
   while(done)b->elements.drop((unsigned char *)stage+(--done)*width,b->elements.context);
#endif
   return result;
  }
 }
 /* Descriptor's relocation contract transfers staged owners by representation
    copy; staging is now dead ownership, not an independently destructible copy. */
 for(size_t i=0;i<count;++i)memcpy((unsigned char *)b->data+(b->count+i)*width,(unsigned char *)stage+i*width,width);
 b->count+=count;return AS_OK;
}
AsStatus as_erased_clear(AsErased *b) {
 if(!descriptor(b))return AS_INVALID;
 while(b->count)b->elements.drop((unsigned char *)b->data+(--b->count)*b->elements.size,b->elements.context);
 return AS_OK;
}
AsStatus as_value_copy_element(void *d,const void *s,void *ctx){(void)ctx;return av_clone(s,d);}
void as_value_drop_element(void *d,void *ctx){(void)ctx;(void)av_drop(d);}
AsStatus as_int_copy_element(void *d,const void *s,void *ctx){(void)ctx;*(int *)d=*(const int *)s;return AS_OK;}
void as_int_drop_element(void *d,void *ctx){(void)d;(void)ctx;}
