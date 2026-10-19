/* Original intentionally defective local teaching target; synthetic values only. */
#include "audit.h"
int audit_packet(const unsigned char*b,size_t n,AuditState*s){
 if(!b||!s||s->present>15||n<3||b[0]!=0xb6||b[1]!=1||b[2]>4)return 0;
 size_t at=3;
 for(unsigned i=0;i<b[2];i++){
  if(n-at<2)return 0;unsigned tag=b[at++],len=b[at++];if(len>n-at)return 0;
  if((tag!=1&&tag!=2)||len!=(tag==1?2u:1u)||b[at]>3)return 0;
  unsigned key=b[at];if(tag==1){s->value[key]=b[at+1];s->present|=(unsigned char)(1u<<key);}else{s->value[key]=0;s->present&=(unsigned char)~(1u<<key);}at+=len;
 }
 return 1;
}
