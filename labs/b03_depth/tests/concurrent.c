#include "depth.h"
#include <stdio.h>
#include <threads.h>
struct b3_context { struct b3_mailbox box; atomic_int failed; };
enum { ITEMS=20000, SPIN_LIMIT=10000000 };
static int producer(void *p){struct b3_context *c=p;int i;unsigned spins=0;for(i=1;i<=ITEMS;i++){
 while(b3_publish(&c->box,i)!=B3_OK){if(atomic_load(&c->failed)||++spins>SPIN_LIMIT){atomic_store(&c->failed,1);return 1;}thrd_yield();}
 }return 0;}
static int consumer(void *p){struct b3_context *c=p;int i,value=0;unsigned spins=0;for(i=1;i<=ITEMS;i++){
 while(b3_take(&c->box,&value)!=B3_OK){if(atomic_load(&c->failed)||++spins>SPIN_LIMIT){atomic_store(&c->failed,1);return 1;}thrd_yield();}
 if(value!=i){atomic_store(&c->failed,1);return 1;}
 }return 0;}
int main(void){struct b3_context c;thrd_t p,q;int a,b;b3_mailbox_init(&c.box);atomic_init(&c.failed,0);
 printf("atomic_uint size=%zu alignment=%zu lock_free=%d; stress=%d values; no weak-memory exhaustiveness claim\n",sizeof(atomic_uint),_Alignof(atomic_uint),atomic_is_lock_free(&c.box.ready),ITEMS);
 if(thrd_create(&p,producer,&c)!=thrd_success)return 2;
 if(thrd_create(&q,consumer,&c)!=thrd_success){atomic_store(&c.failed,1);thrd_join(p,&a);return 2;}
 thrd_join(p,&a);thrd_join(q,&b);
 if(a||b||atomic_load(&c.failed)||atomic_load(&c.box.ready)){fputs("B03_CONCURRENT: protocol or bounded progress failed\n",stderr);return 1;}return 0;}
