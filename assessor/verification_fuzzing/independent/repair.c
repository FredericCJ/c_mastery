#include "audit.h"
#ifndef AUDIT_MUTANT
#define AUDIT_MUTANT 0
#endif
int audit_packet(const unsigned char*b,size_t n,struct audit_state*s){
 if(!b||!s||s->present>15||n<3||b[0]!=0xb6||b[1]!=1||b[2]>4)return 0;
 struct audit_state staged=*s;unsigned seen=0;size_t at=3;
 for(unsigned i=0;i<b[2];i++){
  if(n-at<2)return 0;unsigned tag=b[at++],len=b[at++];if(len>n-at)return 0;
  if((tag!=1&&tag!=2)||len!=(tag==1?2u:1u)||b[at]>3)return 0;
  unsigned key=b[at];if((seen&(1u<<key))&&AUDIT_MUTANT!=1)return 0;seen|=1u<<key;
  if(tag==1){staged.value[key]=b[at+1];staged.present|=(unsigned char)(1u<<key);}else{staged.value[key]=0;staged.present&=(unsigned char)~(1u<<key);}at+=len;
  if(AUDIT_MUTANT==3)*s=staged;
 }
 if(at!=n&&AUDIT_MUTANT!=2)return 0;
 *s=staged;return 1;
}
