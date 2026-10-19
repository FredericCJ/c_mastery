#ifndef CMASTERY_META_CAPACITY_H
#define CMASTERY_META_CAPACITY_H
#include <stddef.h>
#include <stdint.h>
enum { MPC_OK,MPC_INVALID,MPC_RANGE,MPC_FULL,MPC_EMPTY };
typedef struct { uint32_t data[1];size_t head,count; } Mpc1;
typedef struct { uint32_t data[4];size_t head,count; } Mpc4;
typedef struct { uint32_t data[8];size_t head,count; } Mpc8;
int mpc1_push(Mpc1 *,uint32_t);int mpc1_pop(Mpc1 *,uint32_t *);
int mpc4_push(Mpc4 *,uint32_t);int mpc4_pop(Mpc4 *,uint32_t *);
int mpc8_push(Mpc8 *,uint32_t);int mpc8_pop(Mpc8 *,uint32_t *);
/* Zero-init fixed rings. Serialized controls and output/backing disjoint.
   Runtime init borrows capacity live uint32_t elements; budget is bytes.
   Valid capacity is positive, storage multiplication fits size_t and budget.
   Missing pointers INVALID, invalid extent/budget RANGE; failure unchanged.
   Fresh/nonowning control only; no capacity expression is assumed constant. */
typedef struct { uint32_t *data;size_t capacity,head,count,mask;unsigned power2; } MpcRuntime;
int mpc_init(MpcRuntime *,uint32_t *,size_t,size_t);
int mpc_push(MpcRuntime *,uint32_t);
int mpc_pop(MpcRuntime *,uint32_t *);
#endif
