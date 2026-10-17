#include "messaging.h"
#include <stdio.h>
int main(void) {
 char line[64];
 while(fgets(line,sizeof line,stdin)) {
  MsgDeque q; msg_deque_init(&q); unsigned value=0,next=1;
  for(size_t i=0;line[i]&&line[i]!='\n';i++) {
   MsgStatus status;
   if(line[i]=='+')status=msg_push(&q,next++);
   else if(line[i]=='p')status=msg_pop(&q,&value);
   else if(line[i]=='s')status=msg_steal(&q,&value);
   else return 2;
   if(status==MSG_OK&&line[i]!='+')printf("v%u,",value);
   else printf("s%d,",(int)status);
  }
  puts("");
 }
 return ferror(stdin)?1:0;
}
