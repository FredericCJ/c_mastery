#include "facade.h"
BfRecord *bf_mutable(BfRecord *p){(void)p;return 0;}
const BfRecord *bf_const(const BfRecord *p){(void)p;return 0;}
int bf_binary_int(int a,int b){(void)a;(void)b;return 0;}
unsigned bf_binary_unsigned(unsigned a,unsigned b){(void)a;(void)b;return 0;}
int bf_sequence(BfGet a,void *b,BfGet c,void *d,int *e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
