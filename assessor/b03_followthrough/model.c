/* GenMC executes these actual component functions, with bounded attempts. */
#include "follow.h"
#include <pthread.h>
#include <assert.h>
#ifndef BF_MODEL
#define BF_MODEL 0
#endif
static BfBox box;
static B3Mailbox old;
static void *first(void*v){(void)v;
#if BF_MODEL
 (void)b3_publish(&old,1);(void)b3_publish(&old,2);
#else
 (void)bf_put(&box,1);(void)bf_put(&box,3);
#endif
 return 0;}
#if BF_MODEL != 1
static void *second(void*v){(void)v;
#if BF_MODEL == 2
 (void)b3_publish(&old,3);
#else
 (void)bf_put(&box,2);
#endif
 return 0;}
#endif
static void *reader(void*v){int a=0,b=0,ra,rb;(void)v;
#if BF_MODEL
 ra=b3_take(&old,&a);rb=b3_take(&old,&b);
 assert(ra!=B3_OK||(a>=1&&a<=3));assert(rb!=B3_OK||(b>=1&&b<=3));
#else
 ra=bf_get(&box,&a);rb=bf_get(&box,&b);
 assert(ra!=B3_OK||(a>=1&&a<=3));assert(rb!=B3_OK||(b>=1&&b<=3));
#endif
 assert(ra!=B3_OK||rb!=B3_OK||a!=b);return 0;}
int main(void){pthread_t a,b;
#if BF_MODEL != 1
 pthread_t c;
#endif
 bf_box_init(&box);b3_mailbox_init(&old);pthread_create(&a,0,first,0);
#if BF_MODEL != 1
 pthread_create(&c,0,second,0);
#endif
 pthread_create(&b,0,reader,0);pthread_join(a,0);pthread_join(b,0);
#if BF_MODEL != 1
 pthread_join(c,0);
#endif
 return 0;}
