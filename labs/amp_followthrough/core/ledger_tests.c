#include "ledger.h"
#include <stdio.h>
#define C(x) do{if(!(x)){fprintf(stderr,"AF_LEDGER_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){struct af_ledger s={.epoch=1};for(unsigned id=1;id<=100;id++)C(af_admit(&s,1,id)==AF_OK);
 for(unsigned id=1;id<=90;id++)C(af_terminal(&s,1,id,AF_DONE)==AF_OK);
 /* Five items are explicitly dropped AFTER admission, not five rejected
    admissions. The exact original criterion counts 100 accepted inputs. */
 for(unsigned id=91;id<=95;id++)C(af_terminal(&s,1,id,AF_DROPPED)==AF_OK);
 C(af_ledger_restart(&s,0,1)==AF_BUSY&&s.epoch==1&&s.item[95].state==AF_ACCEPTED);
 C(af_ledger_restart(&s,1,0)==AF_BUSY);C(af_ledger_restart(&s,1,1)==AF_OK&&s.epoch==2);
 unsigned done=0,dropped=0,uncertain=0;for(unsigned i=0;i<100;i++){done+=s.item[i].state==AF_DONE;dropped+=s.item[i].state==AF_DROPPED;uncertain+=s.item[i].state==AF_UNCERTAIN;C(s.item[i].epoch==1);}
 C(done==90&&dropped==5&&uncertain==5&&done+dropped+uncertain==s.last);
 C(af_terminal(&s,1,96,AF_DONE)==AF_STALE&&af_terminal(&s,2,96,AF_DONE)==AF_STALE);
 C(af_admit(&s,2,101)==AF_OK&&af_terminal(&s,1,101,AF_DONE)==AF_STALE);C(af_terminal(&s,2,101,AF_DONE)==AF_OK);C(af_terminal(&s,2,101,AF_DROPPED)==AF_CONFLICT);
 s.epoch=UINT32_MAX;C(af_ledger_restart(&s,1,1)==AF_EXHAUSTED);C(af_admit(&s,UINT32_MAX,129)==AF_INVALID);
 puts("AF_PASS:ledger accepted100 completed90 dropped-after-admission5 uncertain5 epoch1to2 old-ack-refused");return 0;}
