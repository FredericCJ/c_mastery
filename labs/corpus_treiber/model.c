/* Actual implementation is included by the model campaign, not a second model
   of stack transitions. Finite two-participant safety checks, not liveness. */
#include "treiber.h"
#include <pthread.h>
#include <assert.h>
#ifndef TS_MODEL
#define TS_MODEL 1
#endif
static TsStack s;
static uint32_t result[2];
static TsStatus status[2];
static void *actor(void *arg){
 unsigned p=(unsigned)(uintptr_t)arg;
#if TS_MODEL == 1
 assert(ts_push(&s,p+10)==TS_OK);
 status[p]=ts_pop(&s,p,&result[p]);
#else
 if(p==0){TsPop v;status[0]=ts_begin(&s,0,&v);if(status[0]==TS_OK){status[0]=ts_commit(&v,&result[0]);}}
 else{status[1]=ts_pop(&s,1,&result[1]);(void)ts_collect(&s);assert(ts_push(&s,22)==TS_OK);}
#endif
 return 0;
}
int main(void){
 pthread_t a,b;assert(ts_init(&s,3)==TS_OK);
#if TS_MODEL != 1
 assert(ts_push(&s,11)==TS_OK);
#endif
 pthread_create(&a,0,actor,(void*)(uintptr_t)0);pthread_create(&b,0,actor,(void*)(uintptr_t)1);
 pthread_join(a,0);pthread_join(b,0);
#if TS_MODEL == 1
 assert(status[0]==TS_OK&&status[1]==TS_OK);assert(result[0]!=result[1]);assert(result[0]+result[1]==21);
#else
 if(status[0]==TS_OK)assert(result[0]==11||result[0]==22);
 if(status[1]==TS_OK)assert(result[1]==11);
 if(status[0]==TS_OK&&status[1]==TS_OK)assert(result[0]!=result[1]);
#endif
 return 0;
}
