#ifndef ARCHITECTURE_ALU_H
#define ARCHITECTURE_ALU_H
#include <stdint.h>
#include "algo.h"
typedef struct {unsigned sum,carry;} AASlice;
typedef struct {unsigned value,z,n,c_arm,cf_x86,v;unsigned carry[6],bit[5];} AAResult;
enum AAStatus {AA_OK,AA_INVALID};
enum AAStatus aa_slice(unsigned,unsigned,unsigned,AASlice *);
/* Width4/5; op0 add with carry input, op1 subtract with borrow input. */
enum AAStatus aa_alu(unsigned,unsigned,unsigned,unsigned,unsigned,AAResult *);
/* Bits0..4: unsigned less/greater, signed less/greater, equal. */
unsigned aa_relations(const AAResult *);
/* Trusted ISA primitive: actual 32-bit CMP, conditional branches and flags.
 * Bits5..7 are native carry/borrow, overflow, negative. */
unsigned aa_isa_compare(uint32_t,uint32_t);
int aa_test(const char *);
void aa_emit(const char *,const unsigned *,unsigned);
void aa_failure(const char *,unsigned);
#endif
