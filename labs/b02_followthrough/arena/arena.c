#include "arena.h"
void bf_arena_init(BfArena*a){(void)a;}
int bf_arena_reserve(BfArena*a,size_t n,size_t l,BfBorrow*b){(void)a;(void)n;(void)l;(void)b;return B2_FAILED;}
int bf_arena_get(const BfArena*a,BfBorrow b,size_t i,unsigned char*out){(void)a;(void)b;(void)i;(void)out;return B2_FAILED;}
int bf_arena_set(BfArena*a,BfBorrow b,size_t i,unsigned char v){(void)a;(void)b;(void)i;(void)v;return B2_FAILED;}
int bf_arena_mark(BfArena*a,B2Mark*m){(void)a;(void)m;return B2_FAILED;}
int bf_arena_register(BfArena*a,BfClose f,void*c){(void)a;(void)f;(void)c;return B2_FAILED;}
int bf_arena_rollback(BfArena*a,B2Mark m){(void)a;(void)m;return B2_FAILED;}
int bf_arena_commit(BfArena*a,B2Mark m){(void)a;(void)m;return B2_FAILED;}
