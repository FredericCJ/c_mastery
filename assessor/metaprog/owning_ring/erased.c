#include "owning_ring.h"
#ifndef MPO_MUTATION
#define MPO_MUTATION 0
#endif
int mpo_erased_init(struct mpo_erased_ring *r,void *const slots[4],void *scratch,struct mpo_erased_ops ops){
 if(!r||!slots||!scratch||!ops.clone||!ops.drop||!ops.move)return MPO_INVALID;
 for(size_t i=0;i<4;i++){
  if(!slots[i]||slots[i]==scratch)return MPO_INVALID;
  for(size_t j=0;j<i;j++)if(slots[i]==slots[j])return MPO_INVALID;
 }
 struct mpo_erased_ring empty={0};empty.ops=ops;empty.scratch=scratch;
 for(size_t i=0;i<4;i++)empty.slots[i]=slots[i];
 *r=empty;return MPO_OK;
}
int mpo_erased_push(struct mpo_erased_ring *r,const void *input){
 if(!r||!input)return MPO_INVALID;
 if(r->count==4){if(MPO_MUTATION==6){(void)r->ops.clone(r->ops.context,input,r->scratch);r->ops.drop(r->ops.context,r->scratch);}return MPO_FULL;}
 if(r->ops.clone(r->ops.context,input,r->scratch)){
  r->ops.drop(r->ops.context,r->scratch);return MPO_CLONE;
 }
 r->ops.move(r->ops.context,r->slots[(r->head+r->count)%4],r->scratch);r->count++;return MPO_OK;
}
int mpo_erased_pop(struct mpo_erased_ring *r,void *out){
 if(!r||!out)return MPO_INVALID;
 if(!r->count)return MPO_EMPTY;
 r->ops.move(r->ops.context,out,r->slots[r->head]);
 if(MPO_MUTATION!=4)r->head=(r->head+1)%4;
 r->count--;return MPO_OK;
}
void mpo_erased_clear(struct mpo_erased_ring *r){
 while(r->count){r->ops.drop(r->ops.context,r->slots[r->head]);r->head=(r->head+1)%4;r->count--;}
}
