#include "wrappers.h"
#include <limits.h>
_Static_assert(sizeof(AWRecord)==8 && offsetof(AWRecord,rank)==4,"selected Record ABI");
_Static_assert(INT_MAX>=INT32_MAX && INT_MIN<=INT32_MIN,"selected tree key model");
int aw_finite_compare(const AWRecord *a,const AWRecord *b,unsigned descending){
 int rank=(a->rank>b->rank)-(a->rank<b->rank);
 return rank?(descending?-rank:rank):(a->id>b->id)-(a->id<b->id);
}
int aw_callback_compare(AWCompare fn,void *ctx,const AWRecord *a,const AWRecord *b,int *result){return fn(ctx,a,b,result);}
static enum AWStatus ready(const AWOwner *o){
 if(!o || !o->alive)return AW_INVALID;
 if(o->busy)return AW_BUSY;
 if(!o->context || o->context->epoch!=o->context_epoch || o->context->descending!=o->descending)return AW_COMPARE;
 if(o->kind==AW_BORROW)for(unsigned i=0;i<o->count;++i){
  const AWRecord *r=o->bank->pointers[i];
  if(r->id!=o->expected[i].id || r->rank!=o->expected[i].rank)return AW_MUTATED;
 }
 return AW_OK;
}
static enum AWStatus revision(const AWOwner *o){return o->generation==o->limit?AW_EXHAUSTED:AW_OK;}
static void project(const AWOwner *o,AWRecord *r){for(unsigned i=0;i<o->count;++i)r[i]=o->kind==AW_VALUE?o->bank->values[i]:*o->bank->pointers[i];}
static const AWRecord *reference(const AWOwner *o,unsigned i){
#ifdef AW_MUTANT_COPY
 return o->bank->pointers[i];
#else
 return o->kind==AW_BORROW?o->bank->pointers[i]:0;
#endif
}
static enum AWStatus comparisons(AWOwner *o,const AWRecord *r,unsigned n){
 if(o->dispatch==AW_FINITE)return AW_OK;
 o->busy=1;
 for(unsigned i=0;i<n;++i)for(unsigned j=0;j<n;++j){
  int got=0,expected=aw_finite_compare(r+i,r+j,o->descending);
  if(aw_callback_compare(o->compare,o->compare_context,r+i,r+j,&got) || ((got>0)-(got<0))!=expected){o->busy=0;return AW_COMPARE;}
 }
 o->busy=0;return AW_OK;
}
static enum AWStatus replace(AWOwner *o,const AWRecord *values,const AWRecord *const *pointers,unsigned n,unsigned capacity,const CDTree *ids){
 o->busy=1;AWBank *next=o->backing.acquire(o->backing.context);
 if(!next){o->busy=0;return AW_BACKING;}
 /* A producer must lease distinct live typed storage, never the retained bank. */
 if(next==o->bank){o->busy=0;return AW_BACKING;}
 for(unsigned i=0;i<3;++i){next->values[i]=(AWRecord){0,0};next->pointers[i]=0;}
 for(unsigned i=0;i<n;++i){
  if(o->kind==AW_VALUE)next->values[i]=values[i];else next->pointers[i]=pointers[i];
#ifdef AW_MUTANT_COPY
  next->pointers[i]=pointers[i];
#endif
 }
 AWBank *old=o->bank;
 o->bank=next;o->count=n;o->capacity=capacity;o->ids=*ids;
 for(unsigned i=0;i<3;++i)o->expected[i]=i<n?values[i]:(AWRecord){0,0};
#ifndef AW_MUTANT_VIEW
 ++o->generation;
#endif
 if(old){for(unsigned i=0;i<3;++i)old->pointers[i]=0;o->backing.release(o->backing.context,old);}
 o->busy=0;return AW_OK;
}
enum AWStatus aw_init(AWOwner *o,unsigned kind,unsigned capacity,unsigned limit,AWContext *context,unsigned dispatch,AWCompare compare,void *compare_context,AWBacking backing){
 if(!o || kind>AW_BORROW || capacity>3 || !limit || !context || context->descending>1 || dispatch>AW_CALLBACK || (dispatch==AW_CALLBACK && !compare) || !backing.acquire || !backing.release)return AW_INVALID;
 AWOwner fresh={0};cd_tree_init(&fresh.ids,1,0);fresh.kind=kind;fresh.capacity=capacity;fresh.limit=limit;fresh.generation=1;fresh.context=context;fresh.context_epoch=context->epoch;fresh.descending=context->descending;fresh.dispatch=dispatch;fresh.compare=compare;fresh.compare_context=compare_context;fresh.backing=backing;fresh.alive=1;*o=fresh;return AW_OK;
}
enum AWStatus aw_put(AWOwner *o,const AWRecord *r){
 enum AWStatus s=ready(o);if(s!=AW_OK)return s;if(!r)return AW_INVALID;
#ifdef AW_MUTANT_PRECEDENCE
 if(o->count==o->capacity)return AW_FULL;
#endif
 if(cd_tree_find(&o->ids,r->id)>=0)return AW_DUPLICATE;
 if(o->count==o->capacity)return AW_FULL;
 if((s=revision(o))!=AW_OK)return s;
 AWRecord values[4];const AWRecord *pointers[3]={0};project(o,values);values[o->count]=*r;
 if((s=comparisons(o,values,o->count+1))!=AW_OK)return s;
 unsigned at=aw_lower(values,o->count,r,o->descending);
 for(unsigned i=o->count;i>at;--i){values[i]=values[i-1];pointers[i]=reference(o,i-1);}
 for(unsigned i=0;i<at;++i)pointers[i]=reference(o,i);
 values[at]=*r;pointers[at]=r;CDTree ids=o->ids;int slot;
 if(cd_tree_insert(&ids,r->id,0,&slot)!=CD_OK)return AW_BACKING;
#ifdef AW_MUTANT_ROLLBACK
 o->ids=ids;
#endif
 return replace(o,values,pointers,o->count+1,o->capacity,&ids);
}
enum AWStatus aw_remove(AWOwner *o,int32_t id){
 enum AWStatus s=ready(o);if(s!=AW_OK)return s;
 if(cd_tree_find(&o->ids,id)<0)return AW_MISSING;
 if((s=revision(o))!=AW_OK)return s;
 AWRecord values[3];const AWRecord *pointers[3]={0};project(o,values);
 if((s=comparisons(o,values,o->count))!=AW_OK)return s;
 unsigned n=0;for(unsigned i=0;i<o->count;++i)if(values[i].id!=id){values[n]=values[i];pointers[n]=reference(o,i);++n;}
 CDTree ids=o->ids;if(cd_tree_delete(&ids,id)!=CD_OK)return AW_BACKING;
 return replace(o,values,pointers,n,o->capacity,&ids);
}
enum AWStatus aw_reserve(AWOwner *o,unsigned capacity){
 enum AWStatus s=ready(o);if(s!=AW_OK)return s;if(capacity>3)return AW_INVALID;if(capacity<=o->capacity)return AW_OK;
 if((s=revision(o))!=AW_OK)return s;
 AWRecord values[3];const AWRecord *pointers[3]={0};project(o,values);
 if((s=comparisons(o,values,o->count))!=AW_OK)return s;
 for(unsigned i=0;i<o->count;++i)pointers[i]=reference(o,i);
 return replace(o,values,pointers,o->count,capacity,&o->ids);
}
enum AWStatus aw_set_context(AWOwner *o,AWContext *context){
 if(!o || !o->alive || !context || context->descending>1)return AW_INVALID;
 if(o->busy)return AW_BUSY;
 if(o->count)return AW_COMPARE;
 enum AWStatus s=revision(o);if(s!=AW_OK)return s;o->context=context;o->context_epoch=context->epoch;o->descending=context->descending;++o->generation;return AW_OK;
}
enum AWStatus aw_view(const AWOwner *o,unsigned index,AWView *out){
 enum AWStatus s=ready(o);if(s!=AW_OK)return s;if(!out)return AW_INVALID;if(index>=o->count)return AW_MISSING;
 *out=(AWView){o,o->generation,index};return AW_OK;
}
enum AWStatus aw_read(AWView view,AWRecord *out){
 const AWOwner *o=view.owner;if(!o || !out)return AW_INVALID;if(!o->alive)return AW_STALE;if(o->busy)return AW_BUSY;
 if(view.generation!=o->generation || view.index>=o->count)return AW_STALE;
 enum AWStatus s=ready(o);if(s!=AW_OK)return s;
#ifdef AW_MUTANT_COPY
 if(o->kind==AW_VALUE){*out=*o->bank->pointers[view.index];return AW_OK;}
#endif
 *out=o->kind==AW_VALUE?o->bank->values[view.index]:*o->bank->pointers[view.index];return AW_OK;
}
enum AWStatus aw_destroy(AWOwner *o){
 if(!o || !o->alive)return AW_INVALID;
 if(o->busy)return AW_BUSY;
 o->busy=1;
 if(o->bank){for(unsigned i=0;i<3;++i)o->bank->pointers[i]=0;o->backing.release(o->backing.context,o->bank);}
 o->bank=0;o->count=0;o->context=0;o->compare=0;o->compare_context=0;o->backing=(AWBacking){0};o->alive=0;o->busy=0;return AW_OK;
}
