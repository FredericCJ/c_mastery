#ifndef LP_FEATURE
#define LP_FEATURE 0
#endif
#if LP_FEATURE == 1
#include <stdalign.h>
#include <stdint.h>
static alignas(long double) unsigned char bytes[32];
int main(void){return (uintptr_t)bytes % alignof(long double)!=0;}
#elif LP_FEATURE == 2
int main(void){int n=0;return _Generic(n++,int:1,default:0)!=1 || n!=0;}
#elif LP_FEATURE == 3
_Static_assert(sizeof(char)==1,"one C byte");
int main(void){return 0;}
#elif LP_FEATURE == 4
#include <stdatomic.h>
int main(void){atomic_uint n=0;unsigned old=atomic_fetch_add(&n,2);return old!=0 || atomic_load(&n)!=2;}
#elif LP_FEATURE == 5
#include <stdatomic.h>
#include <threads.h>
static atomic_uint count;
static int work(void *unused){(void)unused;for(unsigned i=0;i<10000;i++)atomic_fetch_add(&count,1);return 7;}
int main(void){thrd_t a,b;int x,y;atomic_init(&count,0);if(thrd_create(&a,work,0)!=thrd_success)return 2;if(thrd_create(&b,work,0)!=thrd_success){thrd_join(a,&x);return 2;}if(thrd_join(a,&x)!=thrd_success || thrd_join(b,&y)!=thrd_success)return 3;return x!=7 || y!=7 || atomic_load(&count)!=20000;}
#elif LP_FEATURE == 6
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
int main(void){intmax_t x=INTMAX_MAX;printf("%" PRIdMAX "\n",x);
#ifdef UINT32_MAX
 {uint32_t v=UINT32_MAX;printf("exact32 %" PRIu32 "\n",v);}
#else
 puts("exact32 unavailable; choose uint_least32_t for minimum-width algorithm");
#endif
 return 0;}
#elif LP_FEATURE == 7
#include <stdatomic.h>
struct lp_pair {unsigned long long a,b,c;};
static volatile _Atomic(struct lp_pair) cell;
int main(void){struct lp_pair a={3,7,11};atomic_store(&cell,a);a=atomic_load(&cell);return a.a!=3 || a.b!=7 || a.c!=11;}
#else
#error Select one isolated feature
#endif
