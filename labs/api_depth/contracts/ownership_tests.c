#include "ownership.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"OWNERSHIP-CHECK:%s line%d\n",#x,__LINE__);return 1;}}while(0)
int main(void) {
  AcSession invalid={AC_ACTIVE,0,0,0};CHECK(ac_session_admit(&invalid)==AC_INVALID);
  unsigned failures=0;
  for(unsigned f=0;f<=4;++f) for(int optional=0;optional<=1;++optional) {
    AcLedger l={0};AcSession s={AC_NEW,0,0,0};
    AcStatus r=ac_session_init(&s,&l,f,optional);
    CHECK(r==(f?AC_BACKEND:AC_OK));CHECK(ac_session_admit(&s)==AC_OK);
    CHECK(ac_session_stop(&s,&l)==AC_OK && ac_session_stop(&s,&l)==AC_OK);
    CHECK(!s.mask && !l.held[0] && !l.held[1]);
    for(unsigned i=0;i<2;++i) CHECK(l.acquisitions[i]==l.releases[i]);
    unsigned epoch=s.epoch;
    CHECK(ac_session_restart(&s,&l,1)==AC_OK && s.epoch==epoch && s.mask==1);
    CHECK(ac_session_stop(&s,&l)==AC_OK && s.epoch==epoch+1);
    CHECK(ac_session_destroy(&s)==AC_OK && s.state==AC_DESTROYED);
    CHECK(ac_session_stop(&s,&l)==AC_INVALID);++failures;
  }
  AcLedger l={0};int owned=0;
  for(unsigned id=0;id<4;++id) {
    CHECK(ac_resource_acquire(&l,id,7,1,0)==AC_BACKEND && !l.held[id]);
    CHECK(ac_resource_acquire(&l,id,7,0,1)==AC_BACKEND);
    CHECK(ac_resource_query(&l,id,7,&owned)==AC_OK && owned);
    CHECK(ac_resource_acquire(&l,id,7,0,0)==AC_OK && l.acquisitions[id]==1);
    CHECK(ac_resource_acquire(&l,id,8,0,0)==AC_FULL);
    l.consume_on_error[id]=id%2;
    CHECK(ac_resource_close(&l,id,1)==AC_BACKEND);
    CHECK(l.held[id]==!(id%2) && l.releases[id]==id%2);
    if(l.held[id]) CHECK(ac_resource_close(&l,id,0)==AC_OK);
    CHECK(l.releases[id]==1);
  }
  AcTable t={0};AcToken old[6];
  AcLedger cleanup={0};
  for(unsigned i=0;i<4;++i) {cleanup.consume_on_error[i]=i%2;CHECK(ac_resource_acquire(&cleanup,i,1,0,0)==AC_OK);}
  AcCloseReport closed=ac_close_all(&cleanup,3);
  CHECK(closed.first==AC_BACKEND && closed.retained==1);
  CHECK(ac_resource_close(&cleanup,1,0)==AC_INVALID && cleanup.releases[1]==1);
  closed=ac_close_all(&cleanup,0);CHECK(closed.first==AC_OK && !closed.retained);
  for(unsigned i=0;i<4;++i) CHECK(cleanup.releases[i]==1);
  for(unsigned i=0;i<6;++i) {
    CHECK(ac_table_alloc(&t,(int)i,old+i)==AC_OK);
    int value=99;
    for(unsigned j=0;j<i;++j) CHECK(ac_table_get(&t,old[j],&value)==AC_MISSING && value==99);
    CHECK(ac_table_get(&t,old[i],&value)==AC_OK && value==(int)i);
    CHECK(ac_table_free(&t,old[i])==AC_OK);
  }
  AcToken unchanged={99,99};CHECK(ac_table_alloc(&t,0,&unchanged)==AC_FULL && unchanged.slot==99);
  AcShared parent={1,1,0,{NULL,NULL}},child={1,1,0,{NULL,NULL}};
  CHECK(ac_shared_link(&parent,&child)==AC_OK && child.strong==2);
  CHECK(ac_shared_drop(&child)==AC_OK && child.alive);
  CHECK(ac_shared_drop(&parent)==AC_OK && !child.alive && child.releases==1);
  CHECK(ac_shared_retain(&child)==AC_MISSING); /* weak control storage is still live */
  AcShared a={1,1,0,{NULL,NULL}},b={1,1,0,{NULL,NULL}};
  AcShared limited={1,1,0,{NULL,NULL}};
  CHECK(ac_shared_retain(&limited)==AC_OK && ac_shared_retain(&limited)==AC_OK);
  CHECK(ac_shared_retain(&limited)==AC_FULL && limited.strong==3);
  CHECK(ac_shared_drop(&limited)==AC_OK && ac_shared_drop(&limited)==AC_OK && ac_shared_drop(&limited)==AC_OK && limited.releases==1);
  CHECK(ac_shared_link(&a,&b)==AC_OK && ac_shared_link(&b,&a)==AC_OK);
  CHECK(ac_shared_drop(&a)==AC_OK && ac_shared_drop(&b)==AC_OK && a.alive && b.alive);
  a.children[0]=NULL;CHECK(ac_shared_drop(&b)==AC_OK && !a.alive && !b.alive);
  unsigned leases=0;
  for(unsigned issued=0;issued<256;++issued) for(unsigned age=0;age<=127;++age) {
    AcLeaseOwner o={1,issued,7};int value=99;
    CHECK(ac_lease_read(&o,issued,(issued+age)%256,&value)==(age<=3?AC_OK:AC_MISSING));
    CHECK(value==(age<=3?7:99));o.live=0;
    CHECK(ac_lease_reuse(&o,(issued+age)%256,8)==(age<=3?AC_FULL:AC_OK));
    ++leases;
  }
  FBuffer original={{1,2},2,4};FView view;uint8_t byte=99;
  CHECK(f_view(&original,0,2,&view)==F_OK);
  CHECK(f_append(&original,NULL,0)==F_OK && f_view_read(view,0,&byte)==F_OK);
  CHECK(f_append(&original,original.data,SIZE_MAX)==F_FULL && f_view_read(view,0,&byte)==F_OK);
  CHECK(f_append(&original,original.data,1)==F_OK && f_view_read(view,0,&byte)==F_STALE);
  AcEpochOwner split={{{3,4},2,1},1,1};AcSnapshot snapshot;
  CHECK(ac_epoch_snapshot(&split,&snapshot)==AC_OK);
  CHECK(ac_epoch_update(&split,0,9)==AC_OK);
  CHECK(f_view_read(snapshot.bytes,0,&byte)==F_OK && byte==9);
  byte=99;CHECK(ac_epoch_read(&split,snapshot,0,&byte)==AC_MISSING && byte==99);
  CHECK(ac_epoch_snapshot(&split,&snapshot)==AC_OK);
  CHECK(ac_epoch_reset(&split)==AC_OK && ac_epoch_read(&split,snapshot,0,&byte)==AC_MISSING);
  split.live=0;CHECK(ac_epoch_read(&split,snapshot,0,&byte)==AC_MISSING);
  unsigned matrix=0;
  static const int allowed[6][4]={{1,0,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,1,1},{0,0,0,0}};
  for(unsigned state=0;state<6;++state) for(unsigned op=0;op<4;++op) {
    AcLedger ledger={0};AcSession session={(AcState)state,state==AC_ACTIVE?3u:state==AC_PARTIAL?1u:0u,2,0};
    for(unsigned i=0;i<2;++i) if(session.mask&(1u<<i)) CHECK(ac_resource_acquire(&ledger,i,3,0,0)==AC_OK);
    unsigned count=ledger.count,mask=session.mask;AcState oldstate=session.state;
    AcStatus r=op==0?ac_session_init(&session,&ledger,0,0):op==1?ac_session_stop(&session,&ledger):op==2?ac_session_destroy(&session):ac_session_restart(&session,&ledger,0);
    CHECK(r==(allowed[state][op]?AC_OK:AC_INVALID));
    if(!allowed[state][op]) CHECK(session.state==oldstate && session.mask==mask && ledger.count==count);
    CHECK(ac_session_admit(&session)==AC_OK);++matrix;
  }
  AcSession exhausted={AC_STOPPED,0,31,0};AcLedger empty={0};
  CHECK(ac_session_restart(&exhausted,&empty,0)==AC_FULL && exhausted.state==AC_STOPPED && !empty.count);
  printf("OWNERSHIP-PASS:constructor_paths=%u;lease_cases=%u;generation_allocations=6;state_operations=%u;four resource kinds;acyclic and cyclic references\n",failures,leases,matrix);
  return 0;
}
