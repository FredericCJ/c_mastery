#include "models.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){
 if(argc!=2||strlen(argv[1])>64)return 2;
 struct tm_queue q={0};tm_init(&q);
 for(size_t i=0;argv[1][i];i++){
  unsigned char out=99;enum ta_status s;struct tm_queue old=q;char op=argv[1][i];
  if(op=='0'||op=='1')s=tm_push(&q,(unsigned char)(op-'0'));
  else if(op=='2')s=tm_pop(&q,&out);
  else if(op=='3')s=tm_clear(&q);
  else return 2;
  if(s!=TA_OK&&memcmp(&q,&old,sizeof q)!=0){printf("RESULT storage-preservation %zu\n",i);return 0;}
 }
 puts("RESULT none");return 0;
}
