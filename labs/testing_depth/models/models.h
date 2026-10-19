#ifndef TESTING_MODELS_H
#define TESTING_MODELS_H
#include "architecture.h"
typedef struct {TaQueue core;TaStorage storage;uint64_t revision;} TmQueue;
typedef struct {const TmQueue *owner;const unsigned char *pointer;uint64_t revision;} TmBorrow;
void tm_init(TmQueue *);
TaStatus tm_push(TmQueue *,unsigned char);
TaStatus tm_pop(TmQueue *,unsigned char *);
TaStatus tm_clear(TmQueue *);
TaStatus tm_borrow(const TmQueue *,TmBorrow *);
TaStatus tm_read(TmBorrow,unsigned char *);
enum {TM_OK,TM_SHORT,TM_OPCODE,TM_COUNT,TM_TRAILING,TM_INVALID};
typedef struct {unsigned count;uint16_t values[4];} TmPacket;
/* Canonical wire: opcode1,count0..4, exactly count big-endian uint16 values. */
unsigned tm_decode(const unsigned char *,size_t,TmPacket *);
unsigned tm_reference(const unsigned char *,size_t,int permissive,TmPacket *);
typedef struct {int32_t previous;} TmFir;
/* Outputs are exact half-unit numerators. saturation clamps to [-2,+2]. */
unsigned tm_fir(TmFir *,const int32_t *,size_t,int saturated,int64_t *);
#endif
