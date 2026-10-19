#include "alu.h"
enum AAStatus aa_slice(unsigned a,unsigned b,unsigned c,AASlice*r){(void)a;(void)b;(void)c;(void)r;return AA_INVALID;}
enum AAStatus aa_alu(unsigned w,unsigned op,unsigned a,unsigned b,unsigned c,AAResult*r){(void)w;(void)op;(void)a;(void)b;(void)c;(void)r;return AA_INVALID;}
unsigned aa_relations(const AAResult*r){(void)r;return 99;}
