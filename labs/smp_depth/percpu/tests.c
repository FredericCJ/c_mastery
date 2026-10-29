#include "percpu.h"
#include <stdio.h>
#include <string.h>
#define SPC(x) do{if(!(x)){fprintf(stderr,"SP_CHECK:%d\n",__LINE__);return 1;}}while(0)
int main(void){
 struct sp_layout l={0};SPC(sp_layout(0x100003,300000,4,8192,16,&l)==SP_OK);SPC(l.cpus==4&&l.spans[0].low==0x100010&&sp_validate(l.spans,36)==SP_OK);struct sp_layout old=l;
 SPC(sp_layout(0x100003,128,4,8192,16,&l)==SP_SMALL&&!memcmp(&old,&l,sizeof l));SPC(sp_layout(UINT64_MAX-2,8,1,512,8,&l)==SP_RANGE&&!memcmp(&old,&l,sizeof l));SPC(sp_layout(0,200000,5,8192,16,&l)==SP_INVALID);
 struct sp_span s[2]={{10,20,0,0},{20,30,1,0}};SPC(sp_validate(s,2)==SP_OK);s[1].low=19;SPC(sp_validate(s,2)==SP_ALIAS);s[1]=(struct sp_span){30,40,0,0};SPC(sp_validate(s,2)==SP_INVALID);
 uint64_t frames[]={33,48,64},budget=0;SPC(sp_budget(frames,3,40+8+8,120,16,512+15,&budget)==SP_OK&&budget==878);
 uint64_t previous=budget;frames[0]=UINT64_MAX;SPC(sp_budget(frames,3,40,120,16,0,&budget)==SP_RANGE&&budget==previous);
 frames[0]=12;frames[1]=20;frames[2]=28;SPC(sp_budget(frames,3,32,40,8,0,&budget)==SP_OK&&budget==151);
 SPC(sp_layout(0x200000,100000,4,2048,8,&l)==SP_OK&&sp_validate(l.spans,36)==SP_OK);
 puts("SP_PASS:checked-layout-and-explicit-frame-budgets");return 0;
}
