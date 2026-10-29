#include "epoch.h"
#include <stdio.h>
#include <string.h>
#define C(x) do {if(!(x)){fprintf(stderr,"B06_EPOCH_ASSERT line=%d %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){for(unsigned old=0;old<8;old++){unsigned fresh=(old+1)%8;struct k_epoch s={.current=1},before;C(ke_enter(&s,old));C(ke_requeue(&s,1));C(ke_replace(&s));C(!ke_requeue(&s,1));C(ke_requeue(&s,2));C(ke_enter(&s,fresh));before=s;C(!ke_reclaim(&s)&&memcmp(&s,&before,sizeof s)==0);C(ke_leave(&s,old));C(ke_reclaim(&s)&&s.reader[fresh]==2);for(unsigned n=0;n<64;n++){C(ke_enter(&s,old));C(ke_leave(&s,old));C(s.reader[fresh]==2);}C(ke_leave(&s,fresh));C(ke_close(&s));C(!ke_requeue(&s,2));before=s;C(!ke_enter(&s,0)&&memcmp(&s,&before,sizeof s)==0);C(ke_close(&s)&&memcmp(&s,&before,sizeof s)==0);}
 {struct k_epoch s={.current=UINT32_MAX},b=s;C(!ke_replace(&s)&&!ke_close(&s)&&memcmp(&s,&b,sizeof s)==0);C(!ke_enter(&s,8)&&!ke_leave(&s,8)&&memcmp(&s,&b,sizeof s)==0);}
 puts("B06_EPOCH_PASS old_reader_positions=8 continuous_new_epoch=64 wrap=refused");return 0;}
