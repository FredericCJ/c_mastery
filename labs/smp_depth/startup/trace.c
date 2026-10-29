#include "startup.h"
#include <stdio.h>
int main(void){
 unsigned parallel,seed;char word[32];while(scanf("%u%u%31s",&parallel,&seed,word)==3){
  struct ss_world world={0};world.parallel=parallel;world.slot[0].generation=seed;struct ss_ticket tickets[2]={{0,0},{1,0}};fputs("SS",stdout);
  for(unsigned i=0;word[i]&&word[i]!='-';++i){int r;switch(word[i]){
   case 'B':r=ss_begin(&world,0,1,2,0x8000,0x20000,0x21000,&tickets[0]);break;
   case 'C':r=ss_begin(&world,1,2,6,0x9000,0x22000,0x23000,&tickets[1]);break;
   case 'T':r=ss_timeout(&world,tickets[0]);break;
   case 'A':r=ss_ack(&world,tickets[0],(struct ss_evidence){2,15,0x21000-8});break;
   case 'D':r=ss_ack(&world,tickets[1],(struct ss_evidence){6,15,0x23000-8});break;
   case 'S':r=ss_ack(&world,(struct ss_ticket){0,tickets[0].generation?tickets[0].generation-1:0},(struct ss_evidence){2,15,0x21000-8});break;
   case 'P':r=ss_quiescent(&world,tickets[0],1);break;
   case 'K':r=ss_quiescent(&world,tickets[1],1);break;
   case 'Q':r=ss_quiescent(&world,tickets[0],0);break;
   case 'R':r=ss_release(&world,tickets[0]);break;
   case 'J':r=ss_release(&world,tickets[1]);break;
   default:return 2;
  }printf(" %d",r);}
  printf(" | %u %u %u %u\n",world.slot[0].phase,world.slot[0].generation,world.slot[1].phase,world.slot[1].generation);
 }return 0;
}
