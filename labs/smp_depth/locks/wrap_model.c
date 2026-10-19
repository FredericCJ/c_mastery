#include <pthread.h>
#include <assert.h>
#include <locks.c>
#include <genmc.h>
static SlLock lock;
static unsigned counter;
static pthread_t first;
static void turn(unsigned i) {
  SlGuard guard={0};assert(sl_ticket_reserve(&lock,&guard)==SL_OK);
  assert(sl_ticket_ready(&guard,NULL)==SL_OK);
  if(i==15)counter++;
  assert(sl_leave(&guard,NULL)==SL_OK);
}
static void *turns(void *unused) {
 (void)unused;
 /* Exact16-turn prefix, expanded to avoid treating its loop as a wait loop. */
 turn(0);turn(1);turn(2);turn(3);turn(4);turn(5);turn(6);turn(7);
 turn(8);turn(9);turn(10);turn(11);turn(12);turn(13);turn(14);turn(15);
 return NULL;
}
static void *newcomer(void *unused) {
 (void)unused;
 /* Observation constrains allocation order but adds no synchronizes-with. */
 __VERIFIER_assume(atomic_fetch_add_explicit(&lock.next,0,memory_order_relaxed)==16);
 SlGuard guard={0};assert(sl_ticket_reserve(&lock,&guard)==SL_OK);
 int result=sl_ticket_ready(&guard,NULL);
 if(result==SL_PENDING) {
  /* Keep the queued guard live; joining supplies progress only after the
   * unassisted first probe. It cannot repair a premature successful probe. */
  pthread_join(first,NULL);assert(sl_ticket_ready(&guard,NULL)==SL_OK);
  counter++;assert(sl_leave(&guard,NULL)==SL_OK);
 } else {
  assert(result==SL_OK);counter++;assert(sl_leave(&guard,NULL)==SL_OK);
  pthread_join(first,NULL);
 }
 return NULL;
}
int main(void) {
 pthread_t b;assert(sl_init(&lock,SL_TICKET,1,0)==SL_OK);
 pthread_create(&first,NULL,turns,NULL);pthread_create(&b,NULL,newcomer,NULL);
 pthread_join(b,NULL);assert(counter==2);return 0;
}
