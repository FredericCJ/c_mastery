#include "ownership.h"
#include <limits.h>
static OwStatus atomic_increment(atomic_uint *count) {
 unsigned n=atomic_load_explicit(count,memory_order_relaxed);
 do { if(!n)return OW_DEAD;if(n==UINT_MAX)return OW_FULL; }
 while(!atomic_compare_exchange_weak_explicit(count,&n,n+1,memory_order_acquire,memory_order_relaxed));
 return OW_OK;
}
OwStatus ow_atomic_retain(OwAtomicShared *s) {return s?atomic_increment(&s->strong):OW_INVALID;}
OwStatus ow_atomic_upgrade(OwAtomicShared *s) {return ow_atomic_retain(s);}
OwStatus ow_atomic_downgrade(OwAtomicShared *s) {
 /* Caller owns a strong reference throughout: it pins implicit weak. */
 if(!s || !atomic_load_explicit(&s->strong,memory_order_relaxed))return OW_DEAD;
 return atomic_increment(&s->weak);
}
static void atomic_weak_release(OwAtomicShared *s) {
 if(atomic_fetch_sub_explicit(&s->weak,1,memory_order_release)==1) {
  atomic_thread_fence(memory_order_acquire);
  if(s->release)s->release(s->context);
 }
}
OwStatus ow_atomic_weak_drop(OwAtomicShared *s) {
 /* A live explicit weak reference is a precondition, not a recoverable race. */
 if(!s)return OW_INVALID;
 atomic_weak_release(s);return OW_OK;
}
OwStatus ow_atomic_drop(OwAtomicShared *s) {
 /* A live owned strong reference is a precondition. */
 if(!s)return OW_INVALID;
 if(atomic_fetch_sub_explicit(&s->strong,1,memory_order_release)==1) {
  atomic_thread_fence(memory_order_acquire);
  if(s->destroy)s->destroy(s->context);
  atomic_weak_release(s);
 }
 return OW_OK;
}
