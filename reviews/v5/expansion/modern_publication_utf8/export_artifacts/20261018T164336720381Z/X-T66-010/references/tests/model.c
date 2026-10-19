#include "ownership.h"
#include <assert.h>
#include <pthread.h>
#ifndef ORC_CASE
#define ORC_CASE 1
#endif
#ifndef ORC_RESURRECT
#define ORC_RESURRECT 0
#endif
#ifdef ORC_STRONG_CAS
/* Explicit changed-assumption model: remove only spurious retry failures. */
#undef atomic_compare_exchange_weak_explicit
#define atomic_compare_exchange_weak_explicit atomic_compare_exchange_strong_explicit
#endif
#ifdef ORC_RELAX_RELEASE
#define memory_order_release memory_order_relaxed
#endif
/* Runner supplies the exact selected implementation path as macro tokens. */
#ifdef ORC_PROVIDER
#define ORC_STRING_INNER(x) #x
#define ORC_STRING(x) ORC_STRING_INNER(x)
#include ORC_STRING(ORC_PROVIDER)
#endif
static OwAtomicShared refs;
static atomic_uint destructions,reclamations;
static int payload[2];
static void destroy(void *unused){(void)unused;if(ORC_CASE==2){assert(payload[0]==1);assert(payload[1]==2);}else payload[0]=0;atomic_fetch_add(&destructions,1);}
static void reclaim(void *unused){(void)unused;atomic_fetch_add(&reclamations,1);}
static void *one(void *unused){(void)unused;if(ORC_CASE==2)payload[0]=1;assert(ow_atomic_drop(&refs)==OW_OK);return NULL;}
static void *two(void *unused){(void)unused;if(ORC_CASE==2){payload[1]=2;assert(ow_atomic_drop(&refs)==OW_OK);}else{
 OwStatus status;
 if(ORC_RESURRECT){unsigned n=atomic_load_explicit(&refs.strong,memory_order_relaxed);while(!atomic_compare_exchange_weak_explicit(&refs.strong,&n,n+1,memory_order_acquire,memory_order_relaxed)){}status=OW_OK;}
 else status=ow_atomic_upgrade(&refs);
 if(status==OW_OK){assert(payload[0]==41);assert(atomic_load(&destructions)==0);assert(ow_atomic_drop(&refs)==OW_OK);}else assert(status==OW_DEAD);
 /* The owned weak transfers to the joiner on thread return. */
 }return NULL;}
int main(void){atomic_init(&destructions,0);atomic_init(&reclamations,0);atomic_init(&refs.strong,ORC_CASE==2?2:1);atomic_init(&refs.weak,ORC_CASE==2?1:2);refs.destroy=destroy;refs.release=reclaim;refs.context=NULL;payload[0]=ORC_CASE==2?0:41;pthread_t a,b;assert(pthread_create(&a,NULL,one,NULL)==0);assert(pthread_create(&b,NULL,two,NULL)==0);assert(pthread_join(a,NULL)==0);assert(pthread_join(b,NULL)==0);assert(atomic_load(&refs.strong)==0);assert(atomic_load(&destructions)==1);if(ORC_CASE==1){assert(atomic_load(&refs.weak)==1);assert(atomic_load(&reclamations)==0);assert(ow_atomic_weak_drop(&refs)==OW_OK);}assert(atomic_load(&reclamations)==1);return 0;}
