#include "capacity.h"
#include "metaprog.h"
#ifndef MPC_MUTATION
#define MPC_MUTATION 0
#endif
#define MPC_PREFIX mpc1
#define MPC_RING Mpc1
#define MPC_CAPACITY 1
#include "capacity_template.inc"
#define MPC_PREFIX mpc4
#define MPC_RING Mpc4
#define MPC_CAPACITY 4
#include "capacity_template.inc"
#define MPC_PREFIX mpc8
#define MPC_RING Mpc8
#define MPC_CAPACITY 8
#include "capacity_template.inc"
int mpc_init(MpcRuntime *r,uint32_t *data,size_t capacity,size_t budget){
 if(!r||!data)return MPC_INVALID;
 if(MPC_MUTATION==5)r->capacity=capacity;
 size_t bytes;
 if(!mp_storage_bound(capacity,sizeof *data,MPC_MUTATION==4?SIZE_MAX:budget,&bytes))return MPC_RANGE;
 (void)bytes;MpcRuntime staged={data,capacity,0,0,capacity-1,(capacity&(capacity-1))==0};*r=staged;return MPC_OK;
}
static size_t position(const MpcRuntime *r,size_t distance){
 if(r->power2||MPC_MUTATION==3)return (r->head+distance)&r->mask;
 return r->head>=r->capacity-distance?r->head-(r->capacity-distance):r->head+distance;
}
int mpc_push(MpcRuntime *r,uint32_t value){
 if(!r)return MPC_INVALID;
 if(r->count==r->capacity)return MPC_FULL;
 r->data[position(r,r->count)]=value;r->count++;return MPC_OK;
}
int mpc_pop(MpcRuntime *r,uint32_t *out){
 if(!r||!out)return MPC_INVALID;
 if(!r->count){if(MPC_MUTATION==6)*out=0;return MPC_EMPTY;}
 *out=r->data[r->head];r->head=position(r,1);r->count--;return MPC_OK;
}
