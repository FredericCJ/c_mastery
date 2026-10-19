/* Compiler-facing standard signatures. Valid live nonzero extents and copy
 * nonoverlap remain caller requirements; local fr_* null-zero is no promise
 * about the ISO library's domain. This provider is original curriculum code. */
#include "free.h"
void *memcpy(void *d,const void *s,size_t n){return fr_copy(d,s,n);}
void *memmove(void *d,const void *s,size_t n){return fr_move(d,s,n);}
void *memset(void *d,int v,size_t n){return fr_set(d,(unsigned char)v,n);}
int memcmp(const void *a,const void *b,size_t n){return fr_compare(a,b,n);}
