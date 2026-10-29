#include "registry.h"
#include "generic.h"
#ifndef LR_MUTANT
#define LR_MUTANT 0
#endif
static int next(unsigned *n){++*n;return 3;}
int lr_generic(unsigned scenario,struct lr_generic *out)
{
 struct lr_generic r;typedef int lp_alias;lp_alias a=3;const int c=4;volatile int v=5;int array[3]={2,4,6};const int ca[3]={1,3,5};if(!out || scenario>8)return -1;
 r.selection=0;r.value=0;r.real=0.0;r.evaluated=0;
 switch(scenario){case 0:r.selection=1;r.value=LG_VALUE(a);break;case 1:r.selection=2;r.real=LG_VALUE(2.5);break;case 2:r.selection=1;r.value=LG_VALUE(c)+LG_VALUE(v);break;case 3:r.selection=1;r.value=LG_VALUE(array[0]);break;case 4:r.selection=3;r.value=LG_OBJECT(a)+LG_OBJECT(c);break;case 5:r.selection=4;r.value=LG_OBJECT(array)+LG_OBJECT(ca);break;case 6:r.selection=1;r.value=LG_VALUE(next(&r.evaluated));if(LR_MUTANT==3)(void)next(&r.evaluated);break;case 7:r.selection=1;r.value=lg_int(next(&r.evaluated));break;case 8:r.selection=5;r.value=(int)LG_EXTENDED(9L);break;}
 *out=r;return 0;
}
