#include "ownership.h"
#include <limits.h>
#ifndef OW_MUTANT
#define OW_MUTANT 0
#endif
static int allocator_valid(struct v4_allocator a) { return a.take != NULL && a.release != NULL; }
enum ow_status ow_construct(struct ow_owner *out, struct v4_allocator a, const size_t lengths[3]) {
 if (!out || !lengths || out->count || !allocator_valid(a)) return OW_INVALID;
 for (unsigned i=0;i<3;i++) if (OW_MUTANT!=11 && (!lengths[i] || lengths[i]>SIZE_MAX/sizeof(int32_t))) return OW_INVALID;
 struct ow_owner temporary={{NULL,NULL,NULL},0,a};
 for (unsigned i=0;i<3;i++) {
  temporary.block[i]=a.take(a.context,lengths[i]);
  if (!temporary.block[i]) {
   while (temporary.count) {
    --temporary.count;
    if (OW_MUTANT!=1) a.release(a.context,temporary.block[temporary.count]);
   }
   return OW_NOMEM;
  }
  ++temporary.count;
 }
 *out=temporary; return OW_OK;
}
enum ow_status ow_destroy(struct ow_owner *o) {
 if (!o || o->count>3 || (o->count && !allocator_valid(o->allocator))) return OW_INVALID;
 while(o->count) { --o->count; o->allocator.release(o->allocator.context,o->block[o->count]);o->block[o->count]=NULL; }
 return OW_OK;
}
enum ow_status ow_defer(struct ow_scope *s, ow_cleanup_fn fn, void *ctx, enum ow_when when) {
 if (!s || !fn || s->finishing || when<OW_ALWAYS || when>OW_ON_SUCCESS) return OW_INVALID;
 if(s->count>=8) return OW_FULL;
 s->entries[s->count++]=(struct ow_cleanup_entry){fn,ctx,when}; return OW_OK;
}
enum ow_status ow_finish(struct ow_scope *s, int success) {
 if(!s || s->finishing || s->count>8 || (success!=0 && success!=1)) return OW_INVALID;
 s->finishing=1;
 while(s->count) {
  struct ow_cleanup_entry e=s->entries[--s->count];
  if(OW_MUTANT==12) { e=s->entries[0];for(size_t i=0;i<s->count;i++)s->entries[i]=s->entries[i+1]; }
  if(OW_MUTANT==2 || e.when==OW_ALWAYS || (success && e.when==OW_ON_SUCCESS) || (!success && e.when==OW_ON_ERROR)) e.fn(e.context);
 }
 s->finishing=0;return OW_OK;
}
enum ow_status ow_publish(struct ow_owner *published, struct ow_owner *staged) {
 if(!published || !staged || published==staged || published->count>3 || staged->count!=3 || !allocator_valid(staged->allocator) || (published->count && !allocator_valid(published->allocator))) return OW_INVALID;
 struct ow_owner old=*published; *published=*staged; staged->count=0;
 for(unsigned i=0;i<3;i++) staged->block[i]=NULL;
 return ow_destroy(&old);
}
enum ow_status ow_move(struct ow_owner *dst, struct ow_owner *src) {
 if(!dst || !src || dst==src || (dst->count && OW_MUTANT!=13) || src->count==0 || src->count>3) return OW_INVALID;
 *dst=*src;
 if(OW_MUTANT!=3) { src->count=0; for(unsigned i=0;i<3;i++) src->block[i]=NULL; }
 return OW_OK;
}
enum ow_status ow_consume(struct ow_owner *o) { if(!o || !o->count) return OW_DEAD; return ow_destroy(o); }
static int token_live(const struct ow_borrows *b, struct ow_token t) {
 return b && t.slot<4 && b->loan[t.slot].active && b->loan[t.slot].serial==t.serial;
}
enum ow_status ow_borrow(struct ow_borrows *b,size_t off,size_t len,int writer,struct ow_token *out) {
 if(!b || !out || (writer!=0 && writer!=1) || off>b->extent || len>b->extent-off) return OW_INVALID;
 unsigned free_slot=4;
 for(unsigned i=0;i<4;i++) {
  struct ow_loan *l=&b->loan[i];
  if(l->active && (writer || l->writer) && len && l->length && off<l->offset+l->length && l->offset<off+len && OW_MUTANT!=4) return OW_BUSY;
  if(!l->active && !l->retired && l->serial<UINT32_MAX && free_slot==4) free_slot=i;
 }
 if(free_slot==4) return OW_FULL;
 struct ow_loan *l=&b->loan[free_slot]; ++l->serial;l->active=1;l->writer=(unsigned)writer;l->offset=off;l->length=len;
 *out=(struct ow_token){free_slot,l->serial}; return OW_OK;
}
enum ow_status ow_release(struct ow_borrows *b,struct ow_token t) {
 if(!token_live(b,t)) return OW_STALE;
 struct ow_loan *l=&b->loan[t.slot];l->active=0;if(l->serial==UINT32_MAX)l->retired=1;return OW_OK;
}
enum ow_status ow_split(struct ow_borrows *b,struct ow_token parent,size_t left,struct ow_token children[2]) {
 if(!children || !token_live(b,parent)) return OW_STALE;
 struct ow_loan l=b->loan[parent.slot]; if(!l.writer || left>l.length) return OW_INVALID;
 struct ow_borrows candidate=*b;struct ow_token out[2];
 (void)ow_release(&candidate,parent);
 enum ow_status s=ow_borrow(&candidate,l.offset,left,1,&out[0]);
 if(s==OW_OK)s=ow_borrow(&candidate,l.offset+left,l.length-left,1,&out[1]);
 if(s!=OW_OK){if(OW_MUTANT==14)*b=candidate;return s;}
 *b=candidate;children[0]=out[0];children[1]=out[1];return OW_OK;
}
enum ow_status ow_cell_read(const struct ow_cell *c,struct ow_token t,int32_t *out) {
 if(!c || !out || !token_live(&c->borrows,t))return OW_STALE;
 if(c->borrows.loan[t.slot].offset!=0 || c->borrows.loan[t.slot].length!=1)return OW_INVALID;
 *out=c->value;return OW_OK;
}
enum ow_status ow_cell_write(struct ow_cell *c,struct ow_token t,int32_t value) {
 if(!c || !token_live(&c->borrows,t))return OW_STALE;
 struct ow_loan l=c->borrows.loan[t.slot];if((!l.writer && OW_MUTANT!=15) || l.offset!=0 || l.length!=1)return OW_INVALID;
 c->value=value;return OW_OK;
}
enum ow_status ow_cell_replace(struct ow_cell *c,int32_t value,int32_t *old) {
 if(!c || !old)return OW_INVALID;
 for(unsigned i=0;i<4;i++)if(c->borrows.loan[i].active && OW_MUTANT!=5)return OW_BUSY;
 *old=c->value;c->value=value;return OW_OK;
}
enum ow_status ow_shared_retain(struct ow_shared *s) {
 if(!s || !s->strong)return OW_DEAD;
 if(s->strong==UINT_MAX)return OW_FULL;
 ++s->strong;return OW_OK;
}
enum ow_status ow_shared_downgrade(struct ow_shared *s) {
 if(!s || !s->strong)return OW_DEAD;
 if(s->weak==UINT_MAX)return OW_FULL;
 ++s->weak;return OW_OK;
}
enum ow_status ow_shared_upgrade(struct ow_shared *s) {if(OW_MUTANT==16 && s && !s->strong){s->strong=1;return OW_OK;}return ow_shared_retain(s);}
enum ow_status ow_shared_weak_drop(struct ow_shared *s) {
 if(!s || s->weak<=(s->strong!=0))return OW_INVALID;
 if(--s->weak==0 && s->release)s->release(s->context);
 return OW_OK;
}
enum ow_status ow_shared_drop(struct ow_shared *s) {
 if(!s || !s->strong || !s->weak)return OW_DEAD;
 if(--s->strong==0) {
  if(s->destroy)s->destroy(s->context);
  if(OW_MUTANT!=6) { --s->weak;if(!s->weak && s->release)s->release(s->context); }
 }
 return OW_OK;
}
static enum ow_status atomic_increment(atomic_uint *count) {
 unsigned n=atomic_load_explicit(count,memory_order_relaxed);
 do { if(!n)return OW_DEAD;if(n==UINT_MAX)return OW_FULL; }
 while(!atomic_compare_exchange_weak_explicit(count,&n,n+1,memory_order_acquire,memory_order_relaxed));
 return OW_OK;
}
enum ow_status ow_atomic_retain(struct ow_atomic_shared *s) {return s?atomic_increment(&s->strong):OW_INVALID;}
enum ow_status ow_atomic_upgrade(struct ow_atomic_shared *s) {return ow_atomic_retain(s);}
enum ow_status ow_atomic_downgrade(struct ow_atomic_shared *s) {
 /* Caller owns a strong reference throughout: it pins implicit weak. */
 if(!s || !atomic_load_explicit(&s->strong,memory_order_relaxed))return OW_DEAD;
 return atomic_increment(&s->weak);
}
static void atomic_weak_release(struct ow_atomic_shared *s) {
 if(atomic_fetch_sub_explicit(&s->weak,1,memory_order_release)==1) {
  atomic_thread_fence(memory_order_acquire);
  if(s->release)s->release(s->context);
 }
}
enum ow_status ow_atomic_weak_drop(struct ow_atomic_shared *s) {
 /* A live explicit weak reference is a precondition, not a recoverable race. */
 if(!s)return OW_INVALID;
 atomic_weak_release(s);return OW_OK;
}
enum ow_status ow_atomic_drop(struct ow_atomic_shared *s) {
 /* A live owned strong reference is a precondition. */
 if(!s)return OW_INVALID;
 if(atomic_fetch_sub_explicit(&s->strong,1,memory_order_release)==1) {
  atomic_thread_fence(memory_order_acquire);
  if(s->destroy)s->destroy(s->context);
  atomic_weak_release(s);
 }
 return OW_OK;
}
enum ow_status ow_cow_clone(struct ow_cow *c) {if(!c || !c->refs)return OW_DEAD;if(c->refs==UINT_MAX)return OW_FULL;++c->refs;return OW_OK;}
enum ow_status ow_cow_write(struct ow_cow **owner,struct ow_cow *fresh,size_t index,int32_t value) {
 if(!owner || !*owner)return OW_INVALID;
 struct ow_cow *c=*owner;if(!c->refs || !c->data || index>=c->length || !allocator_valid(c->allocator))return OW_INVALID;
 if(c->refs==1 || OW_MUTANT==7){c->data[index]=value;return OW_OK;}
 if(!fresh || fresh==c || fresh->refs || c->length>SIZE_MAX/sizeof(int32_t))return OW_INVALID;
 if(OW_MUTANT==17)--c->refs;
 int32_t *data=c->allocator.take(c->allocator.context,c->length);if(!data)return OW_NOMEM;
 for(size_t i=0;i<c->length;i++)data[i]=c->data[i];
 data[index]=value;
 *fresh=(struct ow_cow){1,data,c->length,c->allocator};--c->refs;*owner=fresh;return OW_OK;
}
enum ow_status ow_cow_drop(struct ow_cow *c) {
 if(!c || !c->refs)return OW_DEAD;
 if(--c->refs==0){c->allocator.release(c->allocator.context,c->data);c->data=NULL;c->length=0;}
 return OW_OK;
}
enum ow_status ow_pin_init(struct ow_pin *p,const void *address,uint64_t thread) {
 if(!p || !address)return OW_INVALID;
 *p=(struct ow_pin){address,thread,0,0};return OW_OK;
}
enum ow_status ow_pin_check(const struct ow_pin *p,const void *address,uint64_t thread) {
 if(!p || !address || (p->thread!=thread && OW_MUTANT!=18))return OW_INVALID;
 return p->address==address?OW_OK:OW_STALE;
}
enum ow_status ow_pin_submit(struct ow_pin *p,const void *address,uint64_t thread) {
 enum ow_status s=ow_pin_check(p,address,thread);if(s!=OW_OK)return s;if(p->pending)return OW_BUSY;p->pending=1;p->cancel=0;return OW_OK;
}
enum ow_status ow_pin_cancel(struct ow_pin *p,uint64_t thread) {
 if(!p || thread!=p->thread || !p->pending)return OW_INVALID;
 p->cancel=1;return OW_OK;
}
enum ow_status ow_pin_complete(struct ow_pin *p,uint64_t thread) {
 if(!p || thread!=p->thread || !p->pending)return OW_INVALID;
 p->pending=0;p->cancel=0;return OW_OK;
}
enum ow_status ow_pin_rebind(struct ow_pin *p,const void *address,uint64_t thread) {
 if(!p || !address || thread!=p->thread)return OW_INVALID;
 if(p->pending && OW_MUTANT!=8)return OW_BUSY;
 p->address=address;return OW_OK;
}
static int32_t *take(void *ctx,size_t count) {
 struct ow_allocator *a=ctx;
 if(a->attempts==SIZE_MAX){a->exhausted_counters=1;return NULL;}
 ++a->attempts;if((a->attempts==a->fail_at && OW_MUTANT!=19) || !count || count>16)return NULL;
 for(unsigned i=0;i<8;i++)if(!a->used[i]) {
  a->used[i]=count;++a->live;if(a->live>a->peak)a->peak=a->live;
  for(size_t j=0;j<count;j++)a->storage[i][j]=INT32_MAX;
  return a->storage[i];
 }
 return NULL;
}
static void release(void *ctx,int32_t *ptr) {
 struct ow_allocator *a=ctx;
 for(unsigned i=0;i<8;i++)if(ptr==a->storage[i] && a->used[i]) {
  if(OW_MUTANT!=9)for(size_t j=0;j<a->used[i];j++)a->storage[i][j]=INT32_MIN;
  a->used[i]=0;--a->live;if(a->releases<SIZE_MAX)++a->releases;else a->exhausted_counters=1;return;
 }
 if(a->invalid_releases<UINT_MAX)++a->invalid_releases;else a->exhausted_counters=1;
}
struct v4_allocator ow_allocator(struct ow_allocator *a) {return (struct v4_allocator){a,take,release};}
enum ow_status ow_flawed_write(struct ow_tenant_arena *a,unsigned tenant,unsigned offset,unsigned count,int32_t value) {
 if(!a || tenant>1 || count>8 || offset+count>8)return OW_INVALID;
 /* Deliberate defined wrap and masked access: remains within actual array. */
 for(unsigned i=0;i<count;i++)a->data[(tenant*8u+offset+i)&15u]=value;
 return OW_OK;
}
enum ow_status ow_checked_write(struct ow_tenant_arena *a,unsigned tenant,unsigned offset,unsigned count,int32_t value) {
 if(OW_MUTANT==10)return ow_flawed_write(a,tenant,offset,count,value);
 if(!a || tenant>1 || offset>8 || count>8-offset+(unsigned)(OW_MUTANT==20 && tenant==0 && offset==8))return OW_INVALID;
 for(unsigned i=0;i<count;i++)a->data[tenant*8u+offset+i]=value;
 return OW_OK;
}
