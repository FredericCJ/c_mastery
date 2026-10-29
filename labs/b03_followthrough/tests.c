#include "follow.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FOLLOW_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static long hot(void*c,const unsigned char*p,size_t n){unsigned *calls=c;(void)p;(void)n;(*calls)++;return -1;}
static long cold(void*c,const unsigned char*p,size_t n){unsigned *calls=c;(void)p;(*calls)++;return (long)n;}
struct bf_halves {unsigned values[9],at;};
static unsigned half(void*c,unsigned high){struct bf_halves*h=c;(void)high;return h->values[h->at++];}
int main(void){
 unsigned char bytes[]={2,3,5};unsigned a=0,b=0,which=9,out=777;b3_write_fn callbacks[2]={hot,cold};void *contexts[2]={&a,&b};
 struct bf_round round={{{bytes,3,0,0},{bytes,3,0,0}},0};struct bf_message m={0};struct bf_operation o={0};struct bf_register r={15,0,0,0};struct bf_box box;int value=99;uint16_t tick=999;struct bf_halves h={{0,0,1,1,1,1,0,0,0},0};
 CHECK(bf_turn(&round,callbacks,contexts,2,&which)==B3_YIELD&&which==0&&a==2);
 CHECK(bf_turn(&round,callbacks,contexts,2,&which)==B3_OK&&which==1&&b==1);
 CHECK(bf_message(&m,1,bytes,3,1,1,&out)==B3_WAIT&&m.done==1&&m.total==2&&out==777);
 CHECK(bf_message(&m,1,bytes,3,3,1,&out)==B3_WAIT&&m.total==10&&m.effects==3&&out==777);
 CHECK(bf_message(&m,1,bytes,3,3,0,&out)==B3_OK&&out==10&&m.effects==3);
 CHECK(bf_start(&o)==B3_OK);
 CHECK(bf_event(&o,1,BF_TIMEOUT)==B3_WAIT);
 CHECK(bf_event(&o,1,BF_REJECT)==B3_WAIT&&o.owned&&o.phase==BF_ACTIVE);
 CHECK(bf_event(&o,1,BF_TIMEOUT)==B3_WAIT);
 CHECK(bf_event(&o,1,BF_QUEUED_ACK)==B3_WAIT&&o.owned&&o.reports==0);
 CHECK(bf_event(&o,1,BF_QUIESCENT)==B3_OK&&!o.owned&&o.reports==1);
 CHECK(bf_w0c(&r,5)==B3_OK&&r.status==10&&r.last_write==250&&r.writes==1&&r.reads==0);
 CHECK(bf_clear_read(&r,&out)==B3_OK&&out==10&&!r.status&&r.reads==1);
 CHECK(bf_clear_read(&r,&out)==B3_OK&&!out&&r.reads==2);
 CHECK(bf_coherent(half,&h,2,&tick)==B3_OK&&tick==257&&h.at==6);
 CHECK(bf_due16(4,65530,10,&value)==B3_OK&&value==1);
 CHECK(bf_ticks(127,1,2,&out)==B3_OK&&out==254);
 bf_box_init(&box);CHECK(bf_put(&box,42)==B3_OK);CHECK(bf_put(&box,43)==B3_WAIT);
 CHECK(bf_get(&box,&value)==B3_OK&&value==42);CHECK(bf_get(&box,&value)==B3_WAIT&&value==42);
 puts("FOLLOW_PASS scheduler receipt queued_ack w0c coherent claim");return 0;
}
