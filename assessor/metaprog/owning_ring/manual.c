#include "owning_ring.h"
int mpo_manual_init(MpoBlobRing *r,MpoBlobOps ops){
 if(!r||!ops.clone||!ops.drop)return MPO_INVALID;
 MpoBlobRing empty={0};empty.ops=ops;*r=empty;return MPO_OK;
}
int mpo_manual_push(MpoBlobRing *r,const MpoBlob *input){
 if(!r||!input)return MPO_INVALID;
 if(r->count==4)return MPO_FULL;
 MpoBlob staged={0};
 if(r->ops.clone(r->ops.context,input,&staged)){r->ops.drop(r->ops.context,&staged);return MPO_CLONE;}
 r->data[(r->head+r->count)%4]=staged;r->count++;return MPO_OK;
}
int mpo_manual_pop(MpoBlobRing *r,MpoBlob *out){
 if(!r||!out)return MPO_INVALID;
 if(!r->count)return MPO_EMPTY;
 *out=r->data[r->head];r->data[r->head]=(MpoBlob){0};r->head=(r->head+1)%4;r->count--;return MPO_OK;
}
void mpo_manual_clear(MpoBlobRing *r){
 while(r->count){r->ops.drop(r->ops.context,&r->data[r->head]);r->head=(r->head+1)%4;r->count--;}
}
