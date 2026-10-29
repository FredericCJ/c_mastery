#ifndef ARCHITECTURE_ALU_H
#define ARCHITECTURE_ALU_H
#include <stdint.h>
#include "algo.h"

struct aa_slice
{
    unsigned sum, carry;
};

struct aa_result
{
    unsigned value, z, n, c_arm, cf_x86, v;
    unsigned carry[6], bit[5];
};

enum aa_status
{
    AA_OK,
    AA_INVALID
};
enum aa_status aa_slice(unsigned, unsigned, unsigned, struct aa_slice *);
/* Width4/5; op0 add with carry input, op1 subtract with borrow input. */
enum aa_status aa_alu(unsigned, unsigned, unsigned, unsigned, unsigned,
                      struct aa_result *);
/* Bits0..4: unsigned less/greater, signed less/greater, equal. */
unsigned aa_relations(const struct aa_result *);
/* Trusted ISA primitive: actual 32-bit CMP, conditional branches and flags.
 * Bits5..7 are native carry/borrow, overflow, negative. */
unsigned aa_isa_compare(uint32_t, uint32_t);
int aa_test(const char *);
void aa_emit(const char *, const unsigned *, unsigned);
void aa_failure(const char *, unsigned);
#endif
