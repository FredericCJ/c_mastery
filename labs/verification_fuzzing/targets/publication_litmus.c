#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
struct vt_state {atomic_uint submitted,payload,published;unsigned observed;};
static void*producer(void*v){struct vt_state*s=v;atomic_store_explicit(&s->submitted,1,memory_order_relaxed);atomic_store_explicit(&s->payload,19,memory_order_relaxed);atomic_store_explicit(&s->published,1,memory_order_release);return NULL;}
static void*consumer(void*v){struct vt_state*s=v;while(!atomic_load_explicit(&s->published,memory_order_acquire)){}s->observed=atomic_load_explicit(&s->payload,memory_order_relaxed);return NULL;}
int main(void){for(unsigned i=0;i<256;i++){struct vt_state s={0};pthread_t a,b;if(pthread_create(&b,NULL,consumer,&s)||pthread_create(&a,NULL,producer,&s))abort();pthread_join(a,NULL);pthread_join(b,NULL);if(s.observed!=19||!atomic_load(&s.submitted))abort();}puts("VT_PUBLICATION_PASS ordered_actual256=pass graph_model_separate");return 0;}
