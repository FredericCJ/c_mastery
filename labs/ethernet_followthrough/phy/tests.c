#include "phy.h"
#include "model.h"
void ep_note(const char *);
#define CHECK(x) do{if(!(x)){ep_note("EP_ASSERT: " #x);return 41;}}while(0)
static int finish(struct ep_phy *p){for(unsigned i=0;i<16;i++){int r=ep_step(p);if(r!=ED_BUSY)return r;}return ED_BUSY;}
int ep_access(void){
 struct ep_model m;ep_model_setup(&m,0);struct ed_bus b=ep_model_bus(&m);uint32_t word=99,div=99;
 CHECK(ed_mdio_word(168000000,0,2,0,&word,&div)==ED_OK && word==0x91 && div==102);
 CHECK(ed_mdio_word(10000000,0,2,0,&word,&div)==ED_UNSUPPORTED && word==0x91 && div==102);
 m.ar=1;m.stuck=1;CHECK(ep_write(&b,168000000,0,0,4,100,0x8000)==ED_BUSY && m.writes==0);
 ep_model_setup(&m,0);CHECK(ep_write(&b,168000000,0,0,4,100,0x8000)==ED_OK && m.reset_value==0x8000 && m.transfers==1 && !m.violation);
 ep_model_setup(&m,0);m.frozen=1;m.stuck=1;CHECK(ep_write(&b,168000000,0,0,3,100,0x8000)==ED_TIMEOUT && m.reads==4 && m.writes==3 && m.transfers==0);
 unsigned writes=m.writes;CHECK(ep_write(&b,168000000,0,0,3,100,0x8000)==ED_BUSY && m.writes==writes);
 for(unsigned fail=1;fail<=6;fail++){ep_model_setup(&m,0);m.fail_at=fail;CHECK(ep_write(&b,168000000,0,0,4,100,0x8000)==ED_IO);}
 ep_model_setup(&m,31);unsigned address=99;CHECK(ep_find(&b,168000000,4,100,&address)==ED_OK && address==31 && m.transfers==64);
 ep_model_setup(&m,31);m.phy[1][2]=7;m.phy[1][3]=0xc130;address=99;CHECK(ep_find(&b,168000000,4,100,&address)==ED_INVALID && address==99);
 ep_model_setup(&m,31);m.phy[31][2]=0xffff;CHECK(ep_find(&b,168000000,4,100,&address)==ED_WRONG_PHY && address==99);
 ep_note("OBS phy_access 145 102 64 3");return 0;
}
int ep_lifecycle(void){
 struct ep_model m;ep_model_setup(&m,0);struct ed_bus b=ep_model_bus(&m);struct ep_phy p={0};struct ep_link old={0},current={0};
 CHECK(ep_init(&p,&b,168000000,0,4,100)==ED_OK && ep_begin(&p,16,1000)==ED_BUSY);
 CHECK(p.incarnation==1 && !p.ready && m.reset_value==0x8000);
 CHECK(ep_step(&p)==ED_BUSY && ep_step(&p)==ED_BUSY && ep_step(&p)==ED_BUSY && p.phase==EP_RESTART);
 CHECK(ep_step(&p)==ED_BUSY && p.phase==EP_RESTART_WAIT && !p.ready);
 CHECK(ep_step(&p)==ED_BUSY && ep_step(&p)==ED_BUSY && p.phase==EP_RESTART_WAIT && !p.ready);
 CHECK(finish(&p)==ED_OK && m.restart_value==0x1200 && p.mode.speed==100 && p.mode.full && ep_snapshot(&p,&old)==ED_OK);
 CHECK(ep_validate(&p,&old)==ED_OK);
 m.latch=1;CHECK(ep_poll(&p)==ED_OK && p.incarnation==2 && !p.mode.historical && p.mode.current && ep_validate(&p,&old)==ED_STALE);
 CHECK(ep_snapshot(&p,&current)==ED_OK && ep_poll(&p)==ED_OK && p.incarnation==2 && ep_validate(&p,&current)==ED_OK);
 m.phy[0][1]=0x20;CHECK(ep_poll(&p)==ED_LINK_DOWN && !p.ready && p.incarnation==3 && ep_validate(&p,&current)==ED_STALE);
 m.phy[0][1]=0x24;m.phy[0][31]=0x1008;CHECK(ep_begin(&p,16,1000)==ED_BUSY && finish(&p)==ED_UNSUPPORTED && !p.ready);
 m.phy[0][31]=0x1014;CHECK(ep_begin(&p,16,1000)==ED_BUSY && finish(&p)==ED_OK && p.mode.speed==10);
 CHECK(ep_snapshot(&p,&current)==ED_OK);p.incarnation=UINT32_MAX;CHECK(ep_invalidate(&p)==ED_FULL && !p.ready && ep_snapshot(&p,&old)==ED_STALE);
 ep_note("OBS phy_lifecycle 32768 4608 100 10");return 0;
}
int ep_bounded(void){
 struct ep_model m;ep_model_setup(&m,0);struct ed_bus b=ep_model_bus(&m);struct ep_phy p={0};m.frozen=1;
 CHECK(ep_init(&p,&b,168000000,0,4,100)==ED_OK && ep_begin(&p,2,1000)==ED_BUSY);
 CHECK(ep_step(&p)==ED_BUSY && ep_step(&p)==ED_BUSY && ep_step(&p)==ED_TIMEOUT && !p.ready && p.remaining==0);
 struct ep_phy q={0};ep_model_setup(&m,0);CHECK(ep_init(&q,&b,168000000,0,4,100)==ED_OK && ep_begin(&q,16,1000)==ED_BUSY);
 m.tick=q.start+1000;CHECK(ep_step(&q)==ED_TIMEOUT && !q.ready);
 struct ep_phy r={0};ep_model_setup(&m,0);CHECK(ep_init(&r,&b,168000000,0,4,100)==ED_OK && ep_begin(&r,16,1000)==ED_BUSY);
 m.frozen=1;m.tick=0;CHECK(ep_step(&r)==ED_TIMEOUT && !r.ready);
 ep_note("OBS phy_bounded 2 1000 0 0");return 0;
}
int ep_faults(void){
 for(unsigned fault=1;fault<=78;fault++){
  struct ep_model m;ep_model_setup(&m,0);m.fail_at=fault;struct ed_bus b=ep_model_bus(&m);struct ep_phy p={0};struct ep_link out={0};
  CHECK(ep_init(&p,&b,168000000,0,4,100)==ED_OK);
  int r=ep_begin(&p,16,1000);if(r==ED_BUSY)r=finish(&p);
  CHECK(r==ED_IO && !p.ready && ep_snapshot(&p,&out)==ED_STALE && !out.owner && !out.incarnation);
 }
 ep_note("OBS phy_faults 78 78 0 0");return 0;
}
