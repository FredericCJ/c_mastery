#include "capacity.h"
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
int mpc_init(MpcRuntime*r,uint32_t*d,size_t n,size_t b){(void)r;(void)d;(void)n;(void)b;return MPC_INVALID;}
int mpc_push(MpcRuntime*r,uint32_t v){(void)r;(void)v;return MPC_INVALID;}
int mpc_pop(MpcRuntime*r,uint32_t*out){(void)r;(void)out;return MPC_INVALID;}
