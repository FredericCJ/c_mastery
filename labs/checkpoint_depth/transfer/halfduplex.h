#ifndef CHECKPOINT_TRANSFER_HALFDUPLEX_H
#define CHECKPOINT_TRANSFER_HALFDUPLEX_H
#include "checkpoint_firmware.h"
typedef void (*CPXDirection)(void *,int);
typedef struct { CPFTx tx;uint16_t turnaround,start;unsigned driving,holding;CPXDirection direction;void *context; } CPXHalfDuplex;
CPFStatus cpx_halfduplex_begin(CPXHalfDuplex *,const CPMessage *,uint16_t,CPXDirection,void *);
CPFStatus cpx_halfduplex_step(CPXHalfDuplex *,uint16_t,int,int,CPFWrite,void *);
#endif
