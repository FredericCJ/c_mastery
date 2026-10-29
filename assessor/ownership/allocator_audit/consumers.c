#include "allocator_audit.h"
static int32_t *up_take(void *context,size_t count){struct v4_allocator a=ow_allocator(context);return a.take(a.context,count);}
static void up_release(void *context,int32_t *p){struct v4_allocator a=ow_allocator(context);a.release(a.context,p);}
static int32_t *up_resize(void *context,int32_t *old,size_t before,size_t after){
 struct v4_allocator a=ow_allocator(context);int32_t *p=a.take(a.context,after);if(!p)return NULL;
 size_t count=before<after?before:after;for(size_t i=0;i<count;++i)p[i]=old[i];a.release(a.context,old);return p;
}
struct oaa_upstream oaa_ow_upstream(struct ow_allocator *a){return (struct oaa_upstream){{a,up_take,up_release},up_resize};}
er_result oaa_parse_call(const unsigned char *s,size_t n,uint32_t *out,oaa_format_fn format,void *context){er_result r=er_parse_pair(s,n,out);if(r.tag!=ER_OK&&format)format(context,r);return r;}
static enum ow_status format_level(struct oaa_formatting *f,er_result r,unsigned depth,char *text,size_t *used){
 ++f->calls;int32_t *scratch=f->allocator.take(f->allocator.context,16);if(!scratch){if(f->failures<SIZE_MAX)++f->failures;return OW_NOMEM;}
 size_t n=0;scratch[n++]='E';scratch[n++]='0'+(int)r.tag;scratch[n++]='@';unsigned value=(unsigned)r.as.offset;int32_t digits[3];unsigned count=0;do{digits[count++]='0'+(int32_t)(value%10);value/=10;}while(value);
 while(count)scratch[n++]=digits[--count];
 for(size_t i=0;i<n;++i)text[(*used)++]=(char)scratch[i];
 enum ow_status status=OW_OK;if(depth>1){text[(*used)++]='[';status=format_level(f,r,depth-1,text,used);text[(*used)++]=']';}
 f->allocator.release(f->allocator.context,scratch);return status;
}
enum ow_status oaa_format_result(struct oaa_formatting *f,er_result r){
 if(!f||!f->allocator.take||!f->allocator.release||!f->depth||f->depth>3||r.tag<=ER_OK||r.tag>ER_NOT_IMPLEMENTED||r.as.offset>999)return OW_INVALID;
 if(f->calls>SIZE_MAX-f->depth)return OW_FULL;
 char stage[64];size_t used=0;enum ow_status status=format_level(f,r,f->depth,stage,&used);if(status==OW_OK){stage[used]=0;for(size_t i=0;i<=used;++i)f->text[i]=stage[i];f->length=used;}f->last=status;return status;
}
void oaa_allocating_formatter(void *p,er_result r){(void)oaa_format_result(p,r);}
