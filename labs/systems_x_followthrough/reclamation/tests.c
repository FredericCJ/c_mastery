#include "list.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LIST_X_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 for(unsigned mode=0;mode<2;mode++){
  struct sx_list l;struct sx_participant a,b,newa;struct sx_observed seen,next;struct sx_protected first,second;unsigned value=999,count=999;
  CHECK(sx_list_init(&l,mode)==MB_OK&&sx_register(&l,&a)==MB_OK&&sx_register(&l,&b)==MB_OK);
  CHECK(sx_append(&l,11)==MB_OK&&sx_append(&l,22)==MB_OK&&sx_append(&l,33)==MB_OK&&sx_append(&l,44)==MB_FULL);
  CHECK(sx_enter(&l,a)==MB_OK&&sx_observe(&l,a,0,&seen)==MB_OK);
  CHECK(sx_protect(&l,a,0,seen,&first)==MB_OK&&sx_read(&l,a,first,&value)==MB_OK&&value==11);
  CHECK(sx_observe(&l,a,first.key,&next)==MB_OK&&sx_protect(&l,a,1,next,&second)==MB_OK);
  CHECK(sx_read(&l,a,second,&value)==MB_OK&&value==22);
  CHECK(sx_depart(&l,a)==MB_BUSY);
  for(unsigned i=1;i<=3;i++)CHECK(sx_remove(&l,&value)==MB_OK&&value==11*i);
  CHECK(sx_collect(&l,&count)==MB_OK&&count==(mode==SX_HAZARD?1u:0u));
  CHECK(sx_read(&l,a,first,&value)==MB_OK&&value==11&&sx_read(&l,a,second,&value)==MB_OK&&value==22);
  if(mode==SX_EPOCH){CHECK(sx_advance(&l)==MB_OK&&sx_advance(&l)==MB_BUSY);}
  CHECK(sx_leave(&l,a)==MB_OK&&sx_read(&l,a,first,&value)==MB_STALE&&sx_depart(&l,a)==MB_OK);
  CHECK(sx_register(&l,&newa)==MB_OK&&newa.slot==a.slot&&newa.generation!=a.generation);
  CHECK(sx_enter(&l,newa)==MB_OK&&sx_clear(&l,a,0)==MB_STALE&&sx_leave(&l,newa)==MB_OK);
  if(mode==SX_EPOCH){CHECK(sx_advance(&l)==MB_OK&&sx_advance(&l)==MB_FULL);}
  CHECK(sx_collect(&l,&count)==MB_OK&&count==(mode==SX_HAZARD?2u:3u));
 }
 {struct sx_list l;struct sx_participant a,b;struct sx_observed old,seen;struct sx_protected protected;unsigned out=999,count;
  CHECK(sx_list_init(&l,SX_HAZARD)==MB_OK&&sx_register(&l,&a)==MB_OK&&sx_register(&l,&b)==MB_OK&&sx_enter(&l,a)==MB_OK&&sx_enter(&l,b)==MB_OK);
  CHECK(sx_append(&l,11)==MB_OK&&sx_observe(&l,a,0,&old)==MB_OK&&sx_remove(&l,&out)==MB_OK&&sx_collect(&l,&count)==MB_OK&&count==1);
  CHECK(sx_append(&l,22)==MB_OK&&sx_protect(&l,a,0,old,&protected)==MB_RETRY);
  CHECK(sx_observe(&l,b,0,&seen)==MB_OK&&sx_protect(&l,b,0,seen,&protected)==MB_OK&&sx_remove(&l,&out)==MB_OK);
  CHECK(sx_collect(&l,&count)==MB_OK&&count==0&&sx_read(&l,b,protected,&out)==MB_OK&&out==22);
  CHECK(sx_clear(&l,b,0)==MB_OK&&sx_read(&l,b,protected,&out)==MB_STALE);
 }
 {struct sx_list l;unsigned out,count;CHECK(sx_list_init(&l,SX_HAZARD)==MB_OK);for(unsigned i=0;i<9;i++){CHECK(sx_append(&l,i)==MB_OK&&sx_remove(&l,&out)==MB_OK&&out==i&&sx_collect(&l,&count)==MB_OK&&count==1);}CHECK(sx_append(&l,9)==MB_FULL);}
 for(unsigned mode=0;mode<2;mode++){struct sx_list l;struct sx_participant p[2];struct sx_observed o;struct sx_protected r[2];unsigned count,value;CHECK(sx_list_init(&l,mode)==MB_OK&&sx_append(&l,11)==MB_OK);for(unsigned i=0;i<2;i++)CHECK(sx_register(&l,&p[i])==MB_OK&&sx_enter(&l,p[i])==MB_OK&&sx_observe(&l,p[i],0,&o)==MB_OK&&sx_protect(&l,p[i],0,o,&r[i])==MB_OK);CHECK(sx_remove(&l,&value)==MB_OK&&sx_collect(&l,&count)==MB_OK&&count==0);CHECK(sx_leave(&l,p[0])==MB_OK&&sx_collect(&l,&count)==MB_OK&&count==0&&sx_read(&l,p[1],r[1],&value)==MB_OK&&value==11);CHECK(sx_leave(&l,p[1])==MB_OK);if(mode==SX_EPOCH)CHECK(sx_advance(&l)==MB_OK&&sx_advance(&l)==MB_OK);CHECK(sx_collect(&l,&count)==MB_OK&&count==1);}
 {struct sx_list l;struct sx_participant p,old;CHECK(sx_list_init(&l,SX_HAZARD)==MB_OK);for(unsigned i=0;i<6;i++){CHECK(sx_register(&l,&p)==MB_OK);old=p;CHECK(sx_depart(&l,p)==MB_OK&&sx_enter(&l,old)==MB_STALE);}CHECK(sx_register(&l,&p)==MB_FULL);}
 {struct sx_list l;struct sx_participant p;struct sx_observed obs;struct sx_protected old,newref;unsigned out=999,count;
  CHECK(sx_list_init(&l,SX_EPOCH)==MB_OK&&sx_register(&l,&p)==MB_OK&&sx_append(&l,11)==MB_OK&&sx_append(&l,22)==MB_OK);
  CHECK(sx_enter(&l,p)==MB_OK&&sx_observe(&l,p,0,&obs)==MB_OK&&sx_protect(&l,p,0,obs,&old)==MB_OK&&sx_leave(&l,p)==MB_OK);
  CHECK(sx_remove(&l,&out)==MB_OK&&sx_advance(&l)==MB_OK&&sx_collect(&l,&count)==MB_OK&&count==0);
  CHECK(sx_advance(&l)==MB_OK&&sx_collect(&l,&count)==MB_OK&&count==1&&sx_enter(&l,p)==MB_OK);
  CHECK(sx_read(&l,p,old,&out)==MB_STALE&&sx_observe(&l,p,0,&obs)==MB_OK&&sx_protect(&l,p,0,obs,&newref)==MB_OK&&sx_read(&l,p,newref,&out)==MB_OK&&out==22);
 }
 {unsigned schedules=0;for(unsigned mask=0;mask<32;mask++){
   unsigned bits=0;for(unsigned i=0;i<5;i++)bits+=(mask>>i)&1u;if(bits!=2)continue;
   struct sx_list l;struct sx_participant p;struct sx_observed obs;struct sx_protected ref;unsigned rstep=0,wstep=0,out=999,count;int observed=0,validated=0;
   CHECK(sx_list_init(&l,SX_HAZARD)==MB_OK&&sx_register(&l,&p)==MB_OK&&sx_append(&l,11)==MB_OK&&sx_enter(&l,p)==MB_OK);
   for(unsigned i=0;i<5;i++)if(mask&(1u<<i)){if(wstep++==0)CHECK(sx_remove(&l,&out)==MB_OK);else CHECK(sx_collect(&l,&count)==MB_OK);}else{
    if(rstep==0){int rc=sx_observe(&l,p,0,&obs);CHECK(rc==MB_OK||rc==MB_EMPTY);observed=rc==MB_OK;}
    if(rstep==1&&observed){int rc=sx_protect(&l,p,0,obs,&ref);CHECK(rc==MB_OK||rc==MB_RETRY);validated=rc==MB_OK;}
    if(rstep==2&&validated)CHECK(sx_read(&l,p,ref,&out)==MB_OK&&out==11);
    rstep++;
   }
   CHECK(sx_leave(&l,p)==MB_OK&&sx_collect(&l,&count)==MB_OK);schedules++;
  }CHECK(schedules==10);
 }
 puts("LIST_X_PASS adjacent2 same_pool3 epoch_distance2 stale_registration no_early_access");return 0;
}
