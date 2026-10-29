#include "response.h"
#include <stdio.h>
int main(void){for(unsigned own=1;own<=4;own++)for(unsigned cost=0;cost<=3;cost++)for(unsigned period=1;period<=9;period++)for(unsigned jitter=0;jitter<=3;jitter++)for(unsigned sw=0;sw<=2;sw++){struct b5_task h={cost,period,jitter};uint32_t out=999;int rc=sx_switch_response(own,0,100,&h,1,sw,&out);printf("R %u %u %u %u %u %d %u\n",own,cost,period,jitter,sw,rc,out);}return 0;}
