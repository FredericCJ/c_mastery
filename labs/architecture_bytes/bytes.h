#ifndef ARCHITECTURE_BYTES_H
#define ARCHITECTURE_BYTES_H
#include <stddef.h>
#include <stdint.h>
#include "wrappers.h"
enum ab_status { AB_OK, AB_INVALID, AB_TRUNCATED, AB_UNSUPPORTED, AB_RANGE };
enum ab_kind { AB_CMP, AB_BRANCH };
struct ab_instruction { unsigned kind, length, operand_bits, reg, condition; uint32_t immediate; int64_t signed_immediate; uint64_t target; };
enum ab_status ab_write(uint64_t,unsigned,uint32_t,uint64_t *);
enum ab_status ab_word(const unsigned char *,size_t,unsigned,uint32_t *);
enum ab_status ab_address(uint64_t,uint64_t,unsigned,int64_t,unsigned,uint64_t,uint64_t *,uint64_t *);
enum ab_status ab_record_decode(const unsigned char *,size_t,unsigned,struct aw_record *);
enum ab_status ab_record_encode(const struct aw_record *,unsigned,unsigned char *,size_t);
enum ab_status ab_record_copy(struct aw_record *,const struct aw_record *);
unsigned ab_thumb_length(uint16_t);
enum ab_status ab_x86(const unsigned char *,size_t,uint64_t,unsigned,struct ab_instruction *);
enum ab_status ab_thumb(const unsigned char *,size_t,uint32_t,unsigned,struct ab_instruction *);
/* Native SysV x86-64 only; original assembly, not arbitrary byte execution. */
void ab_registers(uint64_t [3]);
uint64_t ab_dependency(unsigned,uint64_t,uint64_t,uint64_t);
/* Dedicated Thumb register experiment. */
uint32_t ab_thumb_register(void);
int ab_test(const char *);
void ab_failure(const char *,unsigned);
void ab_emit(const char *,const uint64_t *,unsigned);
#endif
