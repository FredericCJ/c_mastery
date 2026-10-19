#ifndef FC_CHECKS_H
#define FC_CHECKS_H
#include "comparison.h"
#include <limits.h>
static int fc_checks(const FCReport *r){
 unsigned out=91;
 if(!fc_ticks(21,100,&out)||out!=3||!fc_ticks(1,100,&out)||out!=1)return 1;
 if(!fc_ticks(0,100,&out)||out||fc_ticks(UINT_MAX,100,&out)||out)return 2;
 if(fc_ticks(3,0,&out)||fc_ticks(1,10001,&out)||fc_ticks(1,1,0)||out)return 3;
 if(fc_priority(1)!=3||fc_priority(2)!=2||fc_priority(3)!=1||fc_priority(0)||fc_priority(4))return 4;
 if(r->count!=256||r->sum!=32640||!r->full||!r->empty||!r->boosted||!r->restored||!r->closed)return 5;
 if(r->traces!=3||r->trace[0]!=1||r->trace[1]!=2||r->trace[2]!=3)return 6;
 if(!fc_ticks(21,FC_TICK_RATE,&out)||r->ticks!=out||r->elapsed<out)return 7;
#ifdef FC_VENDOR
 if(r->nested_retained!=1)return 8;
#else
 if(r->nested_retained)return 8;
#endif
 return 0;
}
#endif
