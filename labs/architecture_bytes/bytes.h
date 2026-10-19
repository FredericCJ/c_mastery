#ifndef ARCHITECTURE_BYTES_H
#define ARCHITECTURE_BYTES_H
#include <stddef.h>
#include <stdint.h>
#include "wrappers.h"
enum ABStatus { AB_OK, AB_INVALID, AB_TRUNCATED, AB_UNSUPPORTED, AB_RANGE };
enum ABKind { AB_CMP, AB_BRANCH };
typedef struct { unsigned kind, length, operand_bits, reg, condition; uint32_t immediate; int64_t signed_immediate; uint64_t target; } ABInstruction;
enum ABStatus ab_write(uint64_t,unsigned,uint32_t,uint64_t *);
enum ABStatus ab_word(const unsigned char *,size_t,unsigned,uint32_t *);
enum ABStatus ab_address(uint64_t,uint64_t,unsigned,int64_t,unsigned,uint64_t,uint64_t *,uint64_t *);
enum ABStatus ab_record_decode(const unsigned char *,size_t,unsigned,AWRecord *);
enum ABStatus ab_record_encode(const AWRecord *,unsigned,unsigned char *,size_t);
enum ABStatus ab_record_copy(AWRecord *,const AWRecord *);
unsigned ab_thumb_length(uint16_t);
enum ABStatus ab_x86(const unsigned char *,size_t,uint64_t,unsigned,ABInstruction *);
enum ABStatus ab_thumb(const unsigned char *,size_t,uint32_t,unsigned,ABInstruction *);
/* Native SysV x86-64 only; original assembly, not arbitrary byte execution. */
void ab_registers(uint64_t [3]);
uint64_t ab_dependency(unsigned,uint64_t,uint64_t,uint64_t);
/* Dedicated Thumb register experiment. */
uint32_t ab_thumb_register(void);
int ab_test(const char *);
void ab_failure(const char *,unsigned);
void ab_emit(const char *,const uint64_t *,unsigned);
#endif
