#ifndef TESTING_MODELS_H
#define TESTING_MODELS_H
#include "architecture.h"
struct tm_queue {struct ta_queue core;struct ta_storage storage;uint64_t revision;};
struct tm_borrow {const struct tm_queue *owner;const unsigned char *pointer;uint64_t revision;};
void tm_init(struct tm_queue *);
enum ta_status tm_push(struct tm_queue *,unsigned char);
enum ta_status tm_pop(struct tm_queue *,unsigned char *);
enum ta_status tm_clear(struct tm_queue *);
enum ta_status tm_borrow(const struct tm_queue *,struct tm_borrow *);
enum ta_status tm_read(struct tm_borrow,unsigned char *);
enum {TM_OK,TM_SHORT,TM_OPCODE,TM_COUNT,TM_TRAILING,TM_INVALID};
struct tm_packet {unsigned count;uint16_t values[4];};
/* Canonical wire: opcode1,count0..4, exactly count big-endian uint16 values. */
unsigned tm_decode(const unsigned char *,size_t,struct tm_packet *);
unsigned tm_reference(const unsigned char *,size_t,int permissive,struct tm_packet *);
struct tm_fir {int32_t previous;};
/* Outputs are exact half-unit numerators. saturation clamps to [-2,+2]. */
unsigned tm_fir(struct tm_fir *,const int32_t *,size_t,int saturated,int64_t *);
#endif
