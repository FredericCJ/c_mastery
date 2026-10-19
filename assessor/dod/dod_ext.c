#include "dod_ext.h"
#include <limits.h>
#ifndef D_EXT_MUTANT
#define D_EXT_MUTANT 0
#endif
int d_rows_to_columns(const DRow *rows,size_t n,DColumns8 *out) {
 if(!out||(!rows&&n)||n>8)return D_INVALID;
 DColumns8 value={0};value.count=n;
 for(size_t i=0;i<n;++i){
#define D_FIELD(type,name) value.name[i]=rows[i].name;
 D_ROW_FIELDS(D_FIELD)
#undef D_FIELD
 }*out=value;return D_OK;
}
int d_columns_to_rows(const DColumns8 *in,DRow *rows,size_t cap) {
 if(!in||in->count>8||(!rows&&cap))return D_INVALID;
 if(cap<in->count)return D_FULL;
 for(size_t i=0;i<in->count;++i){
#define D_FIELD(type,name) rows[i].name=in->name[i];
 D_ROW_FIELDS(D_FIELD)
#undef D_FIELD
 }return D_OK;
}
int d_rows_step(DRow *rows,size_t n,int32_t ticks) {
 if((!rows&&n)||n>8||ticks<0||ticks>10)return D_INVALID;
 int32_t next[8];
 for(size_t i=0;i<n;++i){int64_t v=(int64_t)rows[i].x+(int64_t)rows[i].velocity*ticks;
  if(v<INT32_MIN||v>INT32_MAX)return D_RANGE;
  next[i]=(int32_t)v;if(D_EXT_MUTANT==1)rows[i].x=next[i];}
 for(size_t i=0;i<n;++i)rows[i].x=next[i];
 return D_OK;
}
int d_columns8_step(DColumns8 *c,int32_t ticks) {
 if(!c||c->count>8)return D_INVALID;
 DRow rows[8];(void)d_columns_to_rows(c,rows,8);
 int status=d_rows_step(rows,c->count,ticks);if(status)return status;
 return d_rows_to_columns(rows,c->count,c);
}
int d_rows_step_many(DRow *rows,size_t n,int32_t ticks){
 if((!rows&&n)||n>1048576||ticks<0||ticks>10)return D_INVALID;
 for(size_t i=0;i<n;++i){int64_t value=(int64_t)rows[i].x+(int64_t)rows[i].velocity*ticks;if(value<INT32_MIN||value>INT32_MAX)return D_RANGE;}
 for(size_t i=0;i<n;++i)rows[i].x=(int32_t)((int64_t)rows[i].x+(int64_t)rows[i].velocity*ticks);
 return D_OK;
}
int d_columns_step_many(int32_t *x,const int32_t *velocity,size_t n,int32_t ticks){
 if(((!x||!velocity)&&n)||n>1048576||ticks<0||ticks>10)return D_INVALID;
 for(size_t i=0;i<n;++i){int64_t value=(int64_t)x[i]+(int64_t)velocity[i]*ticks;if(value<INT32_MIN||value>INT32_MAX)return D_RANGE;}
 for(size_t i=0;i<n;++i)x[i]=(int32_t)((int64_t)x[i]+(int64_t)velocity[i]*ticks);
 return D_OK;
}
int d_rows_to_arrays(const DRow *rows,size_t n,int32_t *x,int32_t *velocity,unsigned *cold,size_t capacity){
 if(n>1048576)return D_INVALID;
 if(capacity<n)return D_FULL;
 if((!rows||!x||!velocity||!cold)&&n)return D_INVALID;
 for(size_t i=0;i<n;++i){
#define D_FIELD(type,name) name[i]=rows[i].name;
 D_ROW_FIELDS(D_FIELD)
#undef D_FIELD
 }return D_OK;
}
int d_arrays_to_rows(const int32_t *x,const int32_t *velocity,const unsigned *cold,size_t n,DRow *rows,size_t capacity){
 if(n>1048576)return D_INVALID;
 if(capacity<n)return D_FULL;
 if((!rows||!x||!velocity||!cold)&&n)return D_INVALID;
 for(size_t i=0;i<n;++i){
#define D_FIELD(type,name) rows[i].name=name[i];
 D_ROW_FIELDS(D_FIELD)
#undef D_FIELD
 }return D_OK;
}
int d_rows_to_tiles(const DRow *rows,size_t n,unsigned cold,DTiles *out) {
 if(!out||(!rows&&n)||n>8||cold>1)return D_INVALID;
 DTiles t={0};t.count=n;t.has_cold=cold;
 for(size_t i=0;i<n;++i){unsigned tile=(unsigned)i/3,lane=(unsigned)i%3;
  t.tiles[tile].x[lane]=rows[i].x;t.tiles[tile].velocity[lane]=rows[i].velocity;
  if(cold)t.tiles[tile].cold[lane]=rows[i].cold;}
 *out=t;return D_OK;
}
int d_tiles_to_rows(const DTiles *t,DRow *out,size_t cap) {
 if(!t||t->count>8||t->has_cold>1||(!out&&cap))return D_INVALID;
 if(cap<t->count)return D_FULL;
 for(size_t i=0;i<t->count;++i){unsigned tile=(unsigned)i/3,lane=(unsigned)i%3;
  if(D_EXT_MUTANT==2 && tile==2)lane=0;
  out[i]=(DRow){t->tiles[tile].x[lane],t->tiles[tile].velocity[lane],t->has_cold?t->tiles[tile].cold[lane]:0};}
 return D_OK;
}
uint32_t d_swar16_add(uint32_t a,uint32_t b) {
 if(D_EXT_MUTANT==3)return a+b;
 uint32_t high=UINT32_C(0x80008000),low=UINT32_C(0x7fff7fff),t=(a&low)+(b&low);
 uint32_t overflow=(a&b&high)|(t&(a|b)&high);
 if(D_EXT_MUTANT==4)overflow=0;
 return (t^((a^b)&high))|((overflow>>15)*65535u);
}
void d_segment_init(DSegment *s,DDestroy destroy,void *context) {
 *s=(DSegment){0};s->destroy=destroy;s->context=context;
 d_sparse_init(&s->pages[0]);d_sparse_init(&s->pages[1]);
}
int d_segment_add(DSegment *s,int value,void *resource,DSegmentHandle *out) {
 if(!s||!out||!resource||!s->destroy)return D_INVALID;
 for(unsigned p=0;p<2;++p){DHandle h;int r=d_sparse_add(&s->pages[p],value,&h);
  if(r==D_FULL)continue;
  if(r)return r;
  s->resource[p][h.slot]=resource;*out=(DSegmentHandle){s,p,h};return D_OK;}
 return D_FULL;
}
static int admitted(const DSegment *s,DSegmentHandle h,int *out) {
 if(!s||!out)return D_INVALID;
 if(h.owner!=s||h.page>=2)return D_STALE;
 return d_sparse_get(&s->pages[h.page],h.handle,out);
}
int d_segment_get(const DSegment *s,DSegmentHandle h,int *out){return admitted(s,h,out);}
int d_segment_borrow(const DSegment *s,DSegmentHandle h,const int **out) {
 if(!out)return D_INVALID;
 int value;int r=admitted(s,h,&value);if(r)return r;
 *out=&s->pages[h.page].slots[h.handle.slot].value;return D_OK;
}
int d_segment_remove(DSegment *s,DSegmentHandle h) {
 int value;int r=admitted(s,h,&value);if(r)return r;
 void *resource=s->resource[h.page][h.handle.slot];
 r=d_sparse_remove(&s->pages[h.page],h.handle);if(r)return r;
 s->resource[h.page][h.handle.slot]=NULL;
 if(D_EXT_MUTANT!=5)s->destroy(s->context,resource);
 if(D_EXT_MUTANT==6)s->pages[h.page].slots[h.handle.slot].generation=h.handle.generation;
 return D_OK;
}
void d_query_world_init(DQueryWorld *w){d_ecs_init(&w->ecs);w->version=1;}
int d_query_set(DQueryWorld *w,unsigned id,unsigned mask,int position,int velocity) {
 if(!w)return D_INVALID;
 if(w->version==UINT_MAX)return D_FULL;
 int r=d_ecs_set(&w->ecs,id,mask,position,velocity);
 if(!r&&D_EXT_MUTANT!=7)++w->version;
 return r;
}
int d_query_get(const DQueryWorld *w,unsigned required,unsigned excluded,DQuery *out) {
 if(!w||!out||required>3||excluded>3||(required&excluded))return D_INVALID;
 if(out->owner==w&&out->version==w->version&&out->required==required&&out->excluded==excluded)return D_OK;
 DQuery q={0};q.owner=w;q.version=w->version;q.required=required;q.excluded=excluded;
 for(unsigned id=0;id<4;++id){unsigned mask=w->ecs.mask[id];
  if(mask&&(mask&required)==required&&(!(mask&excluded)||D_EXT_MUTANT==8))q.ids[q.count++]=id;}
 *out=q;return D_OK;
}
