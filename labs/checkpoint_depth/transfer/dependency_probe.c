#include "checkpoint_memory.h"
#include <stddef.h>
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
int cpx_effect_probe(CPMPool *pool,CPMRing *queue,int (*callback)(int)){
 volatile unsigned char guard[32];guard[0]=7;
 int values[2]={11,22};size_t accepted;
 int status=(int)cpm_prefix(pool,queue,values,2,&accepted);
 return callback(status)+(int)accepted+guard[0];
}
#ifdef CPX_SECRET_ALLOC
/* Resolved forbidden provider: a closed symbol table does not imply purity. */
void *malloc(size_t n){static unsigned char storage[8];return n<=sizeof storage?storage:NULL;}
int cpx_callback(int v){return malloc((size_t)v+1)!=NULL;}
#endif
#ifdef CPX_SECRET_BLOCK
extern int poll(void *,unsigned long,int);
int cpx_callback(int v){return poll(NULL,0,v);}
#endif
