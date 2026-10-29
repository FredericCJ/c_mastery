#include "alu.h"
enum aa_status aa_slice(unsigned a,unsigned b,unsigned c,struct aa_slice*r){(void)a;(void)b;(void)c;(void)r;return AA_INVALID;}
enum aa_status aa_alu(unsigned w,unsigned op,unsigned a,unsigned b,unsigned c,struct aa_result*r){(void)w;(void)op;(void)a;(void)b;(void)c;(void)r;return AA_INVALID;}
unsigned aa_relations(const struct aa_result*r){(void)r;return 99;}
