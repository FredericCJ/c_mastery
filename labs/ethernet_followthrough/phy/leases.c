#include "model.h"
void ep_note(const char *);
#define CHECK(x) do{if(!(x)){ep_note("EP_LEASE_ASSERT: " #x);return 41;}}while(0)
static void barrier(void *p){unsigned *n=p;(*n)++;}
int ep_leases(void){
 EpModel m;ep_model_setup(&m,0);EDBus b=ep_model_bus(&m);EpPhy p={0};EpLink link={0};EDTx tx={0};uint8_t frame[64]={0};unsigned barriers=0;
 CHECK(ep_init(&p,&b,168000000,0,4,100)==ED_OK && ep_begin(&p,16,1000)==ED_BUSY);
 unsigned i=0;int r=ED_BUSY;while(r==ED_BUSY && i++<16)r=ep_step(&p);
 CHECK(r==ED_OK && ep_snapshot(&p,&link)==ED_OK && ep_validate(&p,&link)==ED_OK);
 CHECK(ed_tx_prepare(&tx,0,1,frame,64)==ED_OK && ed_tx_tail(&tx,0,1)==ED_OK && ed_tx_publish(&tx,0,1,barrier,&barriers)==ED_OK);
 frame[0]=0xee;CHECK(tx.slots[0].bytes[0]==0 && barriers==1);
 /* A link-incarnation observation cannot acknowledge MAC DMA ownership. */
 m.phy[0][1]=0x20;CHECK(ep_poll(&p)==ED_LINK_DOWN && ep_validate(&p,&link)==ED_STALE && tx.slots[0].phase==3);
 CHECK(ed_tx_abort(&tx,0,1,0)==ED_BUSY && tx.slots[0].phase==4 && ed_tx_prepare(&tx,0,2,frame,64)==ED_BUSY);
 CHECK(ed_tx_abort(&tx,0,1,1)==ED_OK && ed_tx_prepare(&tx,0,2,frame,64)==ED_OK);
 CHECK(ed_tx_complete(&tx,0,1,1,0)==ED_STALE && tx.slots[0].phase==1);
 CHECK(ed_tx_abort(&tx,0,2,1)==ED_OK);
 ep_note("OBS phy_leases 3 4 1 0");return 0;
}
