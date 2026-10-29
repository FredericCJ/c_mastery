#include "memory.h"
#include <stdio.h>
#ifndef MW_BAD
#define MW_BAD 0
#endif
int main(void){struct mw_async a;mw_async_init(&a);struct mw_ticket first,next;if(mw_submit(&a,19,&first)||mw_terminal(&a,first)||mw_callback_begin(&a,first,0)||mw_submit(&a,23,&next))return 2;unsigned context_live=1;if(MW_BAD)context_live=0;else if(mw_reclaim(&a,first)!=MW_BUSY)return 3;if(!context_live){puts("MW:hidden:bad-caller-reclaims-shared-context-during-successor");return 0;}if(mw_callback_end(&a,first)||mw_submit_return(&a,first)||mw_reclaim(&a,first))return 4;if(!a.slots[next.slot].started)return 5;puts("MW:hidden:fixed-context-held-for-successor:caller-timing-changed");return 0;}
