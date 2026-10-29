#include "depth.h"
#include BF_B02_FACADE
#include "facade.h"
static unsigned calls;
static int get(void *p){(void)p;++calls;return (int)calls;}
volatile int bf_observed;
int bf_target_main(void){struct bf_record r={7};const struct bf_record *c=&r;int i=7,out=0;unsigned u=9;if(B2_IDENTITY(i++)!=7 || i!=8 || B2_IDENTITY(u++)!=9 || u!=10 || BF_BORROW(c)!=&r || bf_sequence(get,0,get,0,&out) || out!=102 || calls!=2)return 1;bf_observed=out;return 0;}
