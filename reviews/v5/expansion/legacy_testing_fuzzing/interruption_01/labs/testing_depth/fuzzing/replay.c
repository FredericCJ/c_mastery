#include "fuzzing.h"
#include <stdio.h>
int main(void){
 unsigned profile,n,value;unsigned char data[16];
 while(scanf("%u %u",&profile,&n)==2){
  if(profile>1||n>16)return 2;
  for(unsigned i=0;i<n;i++){if(scanf("%u",&value)!=1||value>255)return 2;data[i]=(unsigned char)value;}
  TfPacket result={0};unsigned status=tf_parse(profile,data,n,&result);
  printf("%u %u %u %u",status,result.packet.count,result.has_tag,result.tag);
  for(unsigned i=0;i<4;i++)printf(" %u",result.packet.values[i]);puts("");
 }
 return 0;
}
