#include "handles.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"HANDLES-CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 AhPool p;ah_pool_init(&p);AhHandle old[27];unsigned count=0,value=999;
 for(unsigned epoch=1;epoch<=3;++epoch){
  for(unsigned i=0;i<9;++i){AhHandle h;CHECK(ah_allocate(&p,10+i,&h)==AH_OK);CHECK(h.index==i/3&&h.generation==i%3+1&&h.epoch==epoch);old[count++]=h;CHECK(ah_read(&p,h,&value)==AH_OK&&value==10+i);CHECK(ah_release(&p,h)==AH_OK);for(unsigned j=0;j<count;++j){value=999;CHECK(ah_read(&p,old[j],&value)==AH_STALE&&value==999);}}
  AhHandle unchanged={9,9,9},out=unchanged;CHECK(ah_allocate(&p,1,&out)==AH_FULL&&memcmp(&out,&unchanged,sizeof out)==0);
  CHECK(ah_enter(&p)==AH_OK);CHECK(ah_reuse(&p)==AH_BUSY);CHECK(ah_leave(&p)==AH_OK);
  CHECK(ah_reuse(&p)==(epoch==3?AH_FULL:AH_OK));
 }
 CHECK(ah_read(&p,(AhHandle){99,1,3},&value)==AH_STALE);CHECK(ah_leave(&p)==AH_INVALID);
 ah_pool_init(&p);AhHandle slots[3];for(unsigned i=0;i<3;++i)CHECK(ah_allocate(&p,i,&slots[i])==AH_OK);CHECK(ah_reuse(&p)==AH_BUSY);CHECK(ah_allocate(&p,1,&slots[0])==AH_FULL);for(unsigned i=0;i<3;++i)CHECK(ah_release(&p,slots[i])==AH_OK);
 for(unsigned i=0;i<3;++i){CHECK(ah_enter(&p)==AH_OK);}
 CHECK(ah_enter(&p)==AH_FULL);
 for(unsigned i=0;i<3;++i){CHECK(ah_leave(&p)==AH_OK);}
 AhStats stats={0};AhShared *a=0,*b=0;unsigned strong,weak;
 CHECK(ah_shared_create(7,&stats,&a)==AH_OK);CHECK(ah_shared_create(9,&stats,&b)==AH_OK);
 /* The two edge owners are a->b and b->a; roots and edges are distinct obligations. */
 CHECK(ah_retain(a)==AH_OK&&ah_retain(b)==AH_OK);CHECK(ah_drop(a)==AH_OK&&ah_drop(b)==AH_OK);
 CHECK(ah_counts(a,&strong,&weak)==AH_OK&&strong==1&&weak==1);CHECK(ah_counts(b,&strong,&weak)==AH_OK&&strong==1&&weak==1);CHECK(atomic_load(&stats.payload_frees)==0);
 /* The assessor retains the graph's edge capabilities for explicit cycle breaking. */
 CHECK(ah_drop(a)==AH_OK&&ah_drop(b)==AH_OK);CHECK(atomic_load(&stats.payload_frees)==2&&atomic_load(&stats.control_frees)==2);
 CHECK(ah_shared_create(11,&stats,&a)==AH_OK);CHECK(ah_retain(a)==AH_OK&&ah_retain(a)==AH_OK);CHECK(ah_retain(a)==AH_FULL);CHECK(ah_counts(a,&strong,&weak)==AH_OK&&strong==3);
 CHECK(ah_weak_retain(a)==AH_OK&&ah_weak_retain(a)==AH_OK);CHECK(ah_weak_retain(a)==AH_FULL);CHECK(ah_weak_drop(a)==AH_OK);
 CHECK(ah_promote(a)==AH_FULL);CHECK(ah_drop(a)==AH_OK);CHECK(ah_promote(a)==AH_OK);
 CHECK(ah_drop(a)==AH_OK&&ah_drop(a)==AH_OK&&ah_drop(a)==AH_OK);CHECK(ah_counts(a,&strong,&weak)==AH_OK&&strong==0&&weak==1);CHECK(ah_promote(a)==AH_DEAD);CHECK(ah_weak_drop(a)==AH_OK);
 /* One strong a->b edge, one weak b->a edge: dropping roots permits destruction. */
 CHECK(ah_shared_create(1,&stats,&a)==AH_OK&&ah_shared_create(2,&stats,&b)==AH_OK);CHECK(ah_retain(b)==AH_OK&&ah_weak_retain(a)==AH_OK);
 CHECK(ah_drop(b)==AH_OK&&ah_drop(a)==AH_OK);CHECK(ah_promote(a)==AH_DEAD);CHECK(ah_drop(b)==AH_OK&&ah_weak_drop(a)==AH_OK);
 CHECK(atomic_load(&stats.controls)==5&&atomic_load(&stats.payloads)==5&&atomic_load(&stats.control_frees)==5&&atomic_load(&stats.payload_frees)==5);
 printf("HANDLES-PASS:epochs=3;identities=27;cycle_nodes=2;strong_max=3;pool_bytes=%zu;handle_bytes=%zu;\n",sizeof p,sizeof slots[0]);return 0;
}
