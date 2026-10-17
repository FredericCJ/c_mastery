#include "dod.h"
#include <limits.h>
void d_small_init(DSmall *s,DAllocator a){s->heap=0;s->length=0;s->capacity=16;s->allocator=a;}
const unsigned char *d_small_data(const DSmall *s){return s->heap?s->heap:s->local;}
int d_small_truncate(DSmall *s,size_t n){if(!s)return D_INVALID;if(n>s->length)return D_RANGE;s->length=n;return D_OK;}
void d_small_reset(DSmall *s){if(s->heap)s->allocator.release(s->allocator.context,s->heap);s->heap=0;s->length=0;s->capacity=16;}
int d_small_append(DSmall *s,const unsigned char *p,size_t n){size_t i,total,cap;unsigned char *dst;
 if(!s||(!p&&n))return D_INVALID;
#ifdef D_MUTATE_SMALL2
 if(!n)return D_INVALID;
#endif
 if(n>128-s->length)return D_FULL;
 total=s->length+n;
 if(total>s->capacity){
  if(!s->allocator.allocate||!s->allocator.release)return D_FULL;
  cap=s->capacity*2;if(cap<total)cap=total;if(cap>128)cap=128;
  dst=s->allocator.allocate(s->allocator.context,cap);
  if(!dst){
#ifdef D_MUTATE_SMALL
 s->length=total;
#endif
 return D_FAILED;}
  for(i=0;i<s->length;i++)dst[i]=d_small_data(s)[i];
  for(i=0;i<n;i++)dst[s->length+i]=p[i];
  if(s->heap)s->allocator.release(s->allocator.context,s->heap);
  s->heap=dst;s->capacity=cap;s->length=total;return D_OK;
 }
 dst=s->heap?s->heap:s->local;
 for(i=0;i<n;i++)dst[s->length+i]=p[i];
 s->length=total;return D_OK;
}
int d_small_compact(DSmall *s){size_t i;if(!s)return D_INVALID;if(s->length>16)return D_FULL;if(s->heap){for(i=0;i<s->length;i++)s->local[i]=s->heap[i];s->allocator.release(s->allocator.context,s->heap);s->heap=0;s->capacity=16;}return D_OK;}
int d_small_move(DSmall *d,DSmall *s){size_t i;if(!d||!s||d==s||d->length||d->heap)return D_INVALID;d->allocator=s->allocator;d->length=s->length;d->capacity=s->capacity;d->heap=s->heap;if(!s->heap)for(i=0;i<s->length;i++)d->local[i]=s->local[i];s->heap=0;s->length=0;s->capacity=16;return D_OK;}
int d_columns_step(DColumns *c,int32_t ticks){
#ifdef D_MUTATE_COLUMNS
 ticks=0;
#endif
int32_t next[4];size_t i;if(!c||c->count>4||ticks<0||ticks>10)return D_INVALID;for(i=0;i<c->count;i++){int64_t value=(int64_t)c->x[i]+(int64_t)c->velocity[i]*ticks;if(value<INT32_MIN||value>INT32_MAX)return D_RANGE;
#ifdef D_MUTATE_COLUMNS2
 c->x[i]=(int32_t)value;
#endif
next[i]=(int32_t)value;}for(i=0;i<c->count;i++)c->x[i]=next[i];return D_OK;}
uint32_t d_swar_add(uint32_t a,uint32_t b){
#ifdef D_MUTATE_SWAR2
 return a^b;
#elif defined(D_MUTATE_SWAR)
 return a+b;
#else
uint32_t high=UINT32_C(0x80808080),low=UINT32_C(0x7f7f7f7f),t=(a&low)+(b&low);uint32_t overflow=(a&b&high)|(t&(a|b)&high);return (t^((a^b)&high))|((overflow>>7)*255u);
#endif
}
void d_sparse_init(DSparse *s){unsigned i;s->count=0;for(i=0;i<4;i++){s->slots[i]=(DSlot){1,0,0};s->rank[i]=0;s->dense[i]=0;}}
int d_sparse_add(DSparse *s,int value,DHandle *out){unsigned i;if(!s||!out)return D_INVALID;for(i=0;i<4;i++)if(!s->slots[i].live&&s->slots[i].generation<=3){s->slots[i].live=1;s->slots[i].value=value;s->rank[i]=s->count;s->dense[s->count++]=i;*out=(DHandle){s,i,s->slots[i].generation};return D_OK;}return D_FULL;}
static int valid(const DSparse *s,DHandle h){return h.owner==s&&h.slot<4&&s->slots[h.slot].live&&s->slots[h.slot].generation==h.generation;}
int d_sparse_get(const DSparse *s,DHandle h,int *out){if(!s||!out)return D_INVALID;if(!valid(s,h))return D_STALE;*out=s->slots[h.slot].value;return D_OK;}
int d_sparse_remove(DSparse *s,DHandle h){unsigned row,moved;if(!s)return D_INVALID;if(!valid(s,h))return D_STALE;row=s->rank[h.slot];moved=s->dense[--s->count];s->dense[row]=moved;
#ifndef D_MUTATE_SPARSE
 s->rank[moved]=row;
#endif
s->slots[h.slot].live=0;s->slots[h.slot].generation++;
#ifdef D_MUTATE_SPARSE2
 if(s->slots[h.slot].generation==4)s->slots[h.slot].generation=1;
#endif
return D_OK;}
void d_ecs_init(DEcs *e){unsigned i;for(i=0;i<3;i++)e->tables[i].count=0;for(i=0;i<4;i++){e->mask[i]=0;e->row[i]=0;}}
int d_ecs_set(DEcs *e,unsigned id,unsigned mask,int position,int velocity){unsigned old,row,last,moved;DTable *to,*from;if(!e||id>=4||mask<1||mask>3)return D_INVALID;old=e->mask[id];to=&e->tables[mask-1];if(old==mask){row=e->row[id];to->position[row]=position;to->velocity[row]=velocity;return D_OK;}if(to->count==2)return D_FULL;row=to->count;to->ids[row]=id;to->position[row]=position;to->velocity[row]=velocity;to->count++;if(old){from=&e->tables[old-1];last=--from->count;moved=from->ids[last];unsigned r=e->row[id];from->ids[r]=moved;from->position[r]=from->position[last];from->velocity[r]=from->velocity[last];
#ifndef D_MUTATE_ECS
 e->row[moved]=r;
#endif
}e->mask[id]=mask;e->row[id]=row;return D_OK;}
int d_ecs_step(DEcs *e){int temp[2];unsigned i;DTable *t;if(!e)return D_INVALID;t=&e->tables[2];
#ifdef D_MUTATE_ECS2
 t=&e->tables[0];
#endif
for(i=0;i<t->count;i++){long long v=(long long)t->position[i]+t->velocity[i];if(v<INT_MIN||v>INT_MAX)return D_RANGE;temp[i]=(int)v;}for(i=0;i<t->count;i++)t->position[i]=temp[i];return D_OK;}
void d_tree_init(DTree *t){t->used=0;t->fail_after=0;t->allocations=0;}
static unsigned tree_alloc(DTree *t,DTreeNode n){if(t->used==64|| (t->fail_after&&t->allocations==t->fail_after-1))return 0;t->allocations++;t->nodes[++t->used]=n;return t->used;}
static unsigned tree_set(DTree *t,unsigned root,unsigned depth,unsigned index,int value){DTreeNode n={0,0,0};unsigned child;if(root)n=t->nodes[root];if(depth==3)n.value=value;else{if(index&(1u<<(2-depth))){child=tree_set(t,n.right,depth+1,index,value);if(!child)return 0;n.right=child;}else{child=tree_set(t,n.left,depth+1,index,value);if(!child)return 0;n.left=child;}}
#ifdef D_MUTATE_PERSISTENT
 if(root)t->nodes[root]=n;
#endif
 return tree_alloc(t,n);}
int d_tree_update(DTree *t,unsigned root,unsigned index,int value,unsigned *out){unsigned before,result;if(!t||!out||root>t->used||index>=8)return D_INVALID;before=t->used;t->allocations=0;
#ifdef D_MUTATE_PERSISTENT2
 t->fail_after=0;
#endif
result=tree_set(t,root,0,index,value);if(!result){t->used=before;return D_FAILED;}*out=result;return D_OK;}
int d_tree_get(const DTree *t,unsigned root,unsigned index,int *out){unsigned depth;if(!t||!out||root>t->used||index>=8)return D_INVALID;for(depth=0;depth<3&&root;depth++)root=index&(1u<<(2-depth))?t->nodes[root].right:t->nodes[root].left;*out=root?t->nodes[root].value:0;return D_OK;}
void d_hamt_init(DHamt *h){h->used=0;h->fail_after=0;h->allocations=0;}
static unsigned rank4(unsigned mask,unsigned slot){unsigned r=0,i;for(i=0;i<slot;i++)if(mask&(1u<<i))r++;return r;}
static unsigned hamt_alloc(DHamt *h,DHamtNode n){if(h->used==64||(h->fail_after&&h->allocations==h->fail_after-1))return 0;h->allocations++;h->nodes[++h->used]=n;return h->used;}
static unsigned hamt_set(DHamt *h,unsigned root,unsigned depth,unsigned key,int value){DHamtNode n={0,{0},0,{0},{0}};unsigned i,slot,rank,child;if(root)n=h->nodes[root];if(depth==2){for(i=0;i<n.count;i++)if(
#ifdef D_MUTATE_HAMT2
 (n.keys[i]&15)==(key&15)
#else
 n.keys[i]==key
#endif
 ){n.values[i]=value;
#ifdef D_MUTATE_HAMT
 if(root)h->nodes[root]=n;
#endif
 return hamt_alloc(h,n);}if(n.count==2)return 0;n.keys[n.count]=key;n.values[n.count++]=value;}else{slot=(key>>(2*depth))&3;rank=rank4(n.bitmap,slot);child=hamt_set(h,n.bitmap&(1u<<slot)?n.children[rank]:0,depth+1,key,value);if(!child)return 0;if(!(n.bitmap&(1u<<slot))){for(i=n.count;i>rank;i--)n.children[i]=n.children[i-1];n.count++;n.bitmap|=1u<<slot;}n.children[rank]=child;}
#ifdef D_MUTATE_HAMT
 if(root)h->nodes[root]=n;
#endif
 return hamt_alloc(h,n);}
int d_hamt_update(DHamt *h,unsigned root,unsigned key,int value,unsigned *out){unsigned before,result;if(!h||!out||root>h->used||key>=32)return D_INVALID;before=h->used;h->allocations=0;result=hamt_set(h,root,0,key,value);if(!result){h->used=before;return D_FAILED;}*out=result;return D_OK;}
int d_hamt_get(const DHamt *h,unsigned root,unsigned key,int *out){unsigned depth,slot,i;if(!h||!out||root>h->used||key>=32)return D_INVALID;for(depth=0;depth<2;depth++){if(!root)return D_STALE;slot=(key>>(2*depth))&3;if(!(h->nodes[root].bitmap&(1u<<slot)))return D_STALE;root=h->nodes[root].children[rank4(h->nodes[root].bitmap,slot)];}for(i=0;i<h->nodes[root].count;i++)if(h->nodes[root].keys[i]==key){*out=h->nodes[root].values[i];return D_OK;}return D_STALE;}
void d_rope_init(DRope *r){r->used=0;}
int d_rope_leaf(DRope *r,const unsigned char *bytes,size_t n,unsigned *out){unsigned i;if(!r||!out||(!bytes&&n)||n>8)return D_INVALID;if(!n){*out=0;return D_OK;}if(r->used==64)return D_FULL;DRopeNode *p=&r->nodes[r->used+1];p->left=p->right=0;p->length=(unsigned)n;for(i=0;i<n;i++)p->bytes[i]=bytes[i];*out=++r->used;return D_OK;}
int d_rope_concat(DRope *r,unsigned left,unsigned right,unsigned *out){unsigned a,b;if(!r||!out||left>r->used||right>r->used)return D_INVALID;if(!left||!right){*out=left?left:right;return D_OK;}a=r->nodes[left].length;b=r->nodes[right].length;if(b>128-a)return D_FULL;if(r->used==64)return D_FULL;r->nodes[r->used+1]=(DRopeNode){left,right,a+b,{0}};*out=++r->used;return D_OK;}
static void flatten(const DRope *r,unsigned root,unsigned char *out,size_t *position){unsigned i;const DRopeNode *n=&r->nodes[root];if(n->left){
#ifdef D_MUTATE_ROPE
 flatten(r,n->right,out,position);flatten(r,n->left,out,position);
#else
 flatten(r,n->left,out,position);flatten(r,n->right,out,position);
#endif
}else for(i=0;i<n->length;i++)out[(*position)++]=n->bytes[i];}
int d_rope_flatten(const DRope *r,unsigned root,unsigned char *out,size_t cap,size_t *written){size_t position=0;if(!r||!written||root>r->used||(!out&&cap))return D_INVALID;if(root&&r->nodes[root].length>cap){
#ifdef D_MUTATE_ROPE2
 if(cap)out[0]=0;
#endif
 return D_FULL;}if(root)flatten(r,root,out,&position);*written=position;return D_OK;}
