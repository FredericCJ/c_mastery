#include "locks.h"
#include <stdio.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"SL_CHECK:%s:%d\n",#x,__LINE__);return 1;}}while(0)
static uint64_t save(void *p){unsigned *enabled=p;unsigned old=*enabled;*enabled=0;return old;}
static void restore(void *p,uint64_t old){*(unsigned *)p=(unsigned)old;}
int main(void) {
 struct sl_lock lock,other;struct sl_guard guards[16]={{0}};struct sl_stats stats={0};
 CHECK(sl_init(&lock,SL_TICKET,1,14)==SL_OK);
 for(unsigned i=0;i<15;i++){CHECK(sl_ticket_reserve(&lock,&guards[i])==SL_OK);CHECK(guards[i].ticket==((14+i)&15));}
 CHECK(sl_ticket_reserve(&lock,&guards[15])==SL_FULL&&guards[15].phase==0);
 CHECK(sl_ticket_ready(&guards[1],&stats)==SL_PENDING);
 for(unsigned i=0;i<15;i++){CHECK(sl_ticket_ready(&guards[i],&stats)==SL_OK);CHECK(sl_leave(&guards[i],&stats)==SL_OK);}
 CHECK(atomic_load(&lock.admitted)==0&&atomic_load(&lock.serving)==13);
 struct sl_node node;sl_node_init(&node);CHECK(sl_init(&lock,SL_MCS,1,0)==SL_OK);CHECK(sl_init(&other,SL_MCS,2,0)==SL_OK);
 CHECK(sl_enter(&lock,&node,&guards[0],&stats)==SL_OK);
 CHECK(sl_enter(&other,&node,&guards[1],&stats)==SL_BUSY&&guards[1].phase==0&&atomic_load(&other.tail)==NULL);
 CHECK(sl_leave(&guards[0],&stats)==SL_OK);CHECK(sl_enter(&other,&node,&guards[1],&stats)==SL_OK);CHECK(sl_leave(&guards[1],&stats)==SL_OK);
 CHECK(sl_init(&lock,SL_TAS,1,0)==SL_OK);CHECK(sl_init(&other,SL_TAS,2,0)==SL_OK);
 for(unsigned initial=0;initial<2;initial++) {
  unsigned enabled=initial;struct sl_port port={save,restore,&enabled};struct sl_context context={0};struct sl_irq_guard a={0},b={0},bad={0};
  CHECK(sl_irq_enter(&context,port,&lock,NULL,&a,&stats)==SL_OK&&enabled==0&&a.saved==initial);
  CHECK(sl_irq_enter(&context,port,&other,NULL,&b,&stats)==SL_OK&&enabled==0&&b.saved==0);
  CHECK(sl_irq_leave(&context,port,&a,&stats)==SL_ORDER&&enabled==0&&context.depth==2);
  /* Different lower-rank lock avoids a real deadlock even in order mutant. */
  struct sl_lock lower;CHECK(sl_init(&lower,SL_TAS,1,0)==SL_OK);
  int result=sl_irq_enter(&context,port,&lower,NULL,&bad,&stats);CHECK(result==SL_ORDER&&bad.guard.phase==0&&enabled==0);
  CHECK(sl_irq_leave(&context,port,&b,&stats)==SL_OK&&enabled==0&&context.depth==1);
  CHECK(sl_irq_leave(&context,port,&a,&stats)==SL_OK&&enabled==initial&&context.depth==0);
 }
 puts("SL_PASS:ticket-wrap-node-retirement-nested-IRQ-rank");return 0;
}
