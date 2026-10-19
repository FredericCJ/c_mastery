#include "invariants.h"
enum AIStatus ai_step(AIState s,unsigned m,AIState *out){(void)s;(void)m;(void)out;return AI_UNIMPLEMENTED;}
enum AIStatus ai_permutation(const AIRecord *a,const AIRecord *b,size_t n,unsigned *out){(void)a;(void)b;(void)n;(void)out;return AI_UNIMPLEMENTED;}
enum AIStatus ai_growth(unsigned n,unsigned a,AIGrowth *out){(void)n;(void)a;(void)out;return AI_UNIMPLEMENTED;}
void ai_queues_init(AIQueues *q){(void)q;}
enum CDStatus ai_queue_event(AIQueues *q,unsigned p,int v,int *out){(void)q;(void)p;(void)v;(void)out;return CD_INVALID;}
