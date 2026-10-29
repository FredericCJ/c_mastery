#include "escape.h"
#include <stdio.h>
#include <string.h>
int main(void) {
 unsigned op;size_t cap,n;unsigned char input[512];struct es_text out,old;
 while(scanf("%u %zu %zu",&op,&cap,&n)==3) {
  if(n>512)return 2;
  for(size_t i=0;i<n;i++){unsigned x;if(scanf("%u",&x)!=1||x>255)return 2;input[i]=(unsigned char)x;}
  memset(&old,0xa5,sizeof old);out=old;
  int status=op==0?es_encode(input,n,cap,&out):op==1?es_decode(input,n,cap,&out):es_recode_html(input,n,cap,&out);
  printf("%d",status);
  if(status)printf(" %d",memcmp(&out,&old,sizeof out)==0);
  else {printf(" %zu",out.length);for(size_t i=0;i<out.length;i++)printf(" %u",out.bytes[i]);}
  putchar('\n');
 }
 return 0;
}
