#include "dispatch_extensions.h"
#include <limits.h>
#ifndef DS_EXT_MUTANT
#define DS_EXT_MUTANT 0
#endif
static int int_measure(const void *p,int *out){*out=*(const int *)p;return DS_OK;}
static int bytes_measure(const void *p,int *out){const DSBytesReceiver *r=p;if((!r->bytes&&r->length)||r->length>INT_MAX)return DS_INVALID;
#if DS_EXT_MUTANT == 1
 *out=0;
#else
 *out=(int)r->length;
#endif
 return DS_OK;}
static const DSOps int_ops={int_measure},byte_ops={bytes_measure};
DSIntInterface ds_int_interface(const DSIntReceiver *r){return (DSIntInterface){{r?r->object:NULL,&int_ops}};}
DSByteInterface ds_byte_interface(const DSBytesReceiver *r){
#if DS_EXT_MUTANT == 2
 (void)r;return (DSByteInterface){{NULL,&byte_ops}};
#else
 return (DSByteInterface){{r,&byte_ops}};
#endif
}
DSProgress ds_read_progress(DSReader r,unsigned char *out,size_t capacity){
 DSProgress result={DS_INVALID,0};if(!r.read||(!out&&capacity)||capacity>16)return result;
 unsigned char staged[16];size_t count=0;int status=r.read(r.context,staged,capacity,&count);
 if(count>capacity||(status!=DS_OK&&status!=DS_FAILED&&status!=DS_EMPTY)||(status==DS_EMPTY&&count))return result;
 result=(DSProgress){status,count};
#if DS_EXT_MUTANT == 3
 if(status!=DS_OK)return (DSProgress){status,0};
#elif DS_EXT_MUTANT == 4
 result.status=DS_OK;
#endif
 for(size_t i=0;i<count;++i)out[i]=staged[i];
 return result;
}
int ds_borrow_closure_init(DSBorrowClosure *c,const int32_t *p,size_t n,unsigned once){if(!c||(!p&&n)||n<1||n>8||once>1)return DS_INVALID;*c=(DSBorrowClosure){p,n,once,1};return DS_OK;}
int ds_borrow_closure_call(DSBorrowClosure *c,int arg,int *out){if(!c||!out)return DS_INVALID;if(!c->active)return DS_EMPTY;if(c->once)c->active=0;return ds_borrowed_i32(c->values,c->count,arg,out);}
int ds_owned_closure_init(DSOwnedClosure *c,DSResource r){if(!c||!r.object||!r.drop)return DS_INVALID;*c=(DSOwnedClosure){r,1};return DS_OK;}
int ds_owned_closure_call(DSOwnedClosure *c,DSOwnedBody body,int arg,DSOwnedResult *out){
 if(!c||!body||!out||out->retry.object||out->retry.drop)return DS_INVALID;
 if(!c->active)return DS_EMPTY;
 DSResource resource=c->resource;
#if DS_EXT_MUTANT != 5
 c->resource=(DSResource){0};c->active=0;
#endif
 DSOwnedResult result=body(resource,arg);
#if DS_EXT_MUTANT == 6
 if(result.retry.object){result.retry.drop(result.retry.object);result.retry=(DSResource){0};}
#endif
 *out=result;return DS_OK;
}
void ds_owned_closure_drop(DSOwnedClosure *c){if(c->active){DSResource r=c->resource;c->active=0;c->resource=(DSResource){0};r.drop(r.object);}}
static void drop_parts(void *context){DSCloneCapture *c=context;for(unsigned i=2;i;){--i;if(c->parts[i]){c->ops.drop(c->ops.context,c->parts[i]);c->parts[i]=NULL;}}}
void ds_clone_capture_init(DSCloneCapture *c,DAllocator a,DSCloneOps ops){ds_erased_init(&c->erased,a);c->ops=ops;c->parts[0]=c->parts[1]=NULL;}
int ds_clone_capture_adopt(DSCloneCapture *c,const unsigned char *bytes,size_t n,DSThunk call,void *a,void *b){if(!c||!a||!b||a==b||!c->ops.clone||!c->ops.drop||c->parts[0]||c->parts[1])return DS_INVALID;int status=ds_erased_assign(&c->erased,bytes,n,call,drop_parts,c);if(status)return status;c->parts[0]=a;c->parts[1]=b;return DS_OK;}
int ds_clone_capture(DSCloneCapture *dest,const DSCloneCapture *source){
 if(!dest||!source||dest==source||dest->erased.active||dest->parts[0]||dest->parts[1]||!source->erased.active||!source->ops.clone||!source->ops.drop)return DS_INVALID;
 void *parts[2]={0};int status=source->ops.clone(source->ops.context,source->parts[0],&parts[0]);if(status!=DS_OK)return status;
 status=source->ops.clone(source->ops.context,source->parts[1],&parts[1]);if(status!=DS_OK){
#if DS_EXT_MUTANT != 7
  source->ops.drop(source->ops.context,parts[0]);
#endif
  return status;}
 DSCloneOps previous=dest->ops;dest->ops=source->ops;
 status=ds_clone_capture_adopt(dest,d_small_data(&source->erased.capture),source->erased.capture.length,source->erased.call,parts[0],parts[1]);
 if(status!=DS_OK){source->ops.drop(source->ops.context,parts[1]);source->ops.drop(source->ops.context,parts[0]);dest->ops=previous;return status;}
#if DS_EXT_MUTANT == 8
 dest->parts[0]=source->parts[0];
#endif
 return DS_OK;
}
void ds_clone_capture_drop(DSCloneCapture *c){ds_erased_drop(&c->erased);}
