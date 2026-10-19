#include "model.h"
void ep_note(const char *);
#define CHECK(x) do{if(!(x)){ep_note("EP_MAP_ASSERT: " #x);return 41;}}while(0)
static EpModel mapped;
int ep_test_read(uint32_t address,uint32_t *value){
 if(address!=0x40028010u && address!=0x40028014u)return 1;
 return ep_model_read(&mapped,(address-0x40028010u)/4,value);
}
int ep_test_write(uint32_t address,uint32_t value){
 if(address!=0x40028010u && address!=0x40028014u)return 1;
 return ep_model_write(&mapped,(address-0x40028010u)/4,value);
}
int ep_mapped(void){
 ep_model_setup(&mapped,0);EpMmioClock clock={&mapped,ep_model_clock};EDBus b={0};EpPhy p={0};EpLink link={0};
 CHECK(ep_f439_bus(0,&b)==ED_INVALID && !b.read);
 CHECK(ep_f439_bus(&clock,&b)==ED_OK && ep_init(&p,&b,168000000,0,4,100)==ED_OK);
 CHECK(ep_begin(&p,16,1000)==ED_BUSY);
 unsigned i=0;int r=ED_BUSY;while(r==ED_BUSY && i++<16)r=ep_step(&p);
 CHECK(r==ED_OK && ep_snapshot(&p,&link)==ED_OK && link.mode.speed==100);
 CHECK(mapped.reset_value==0x8000 && mapped.restart_value==0x1200 && !mapped.violation);
 mapped.phy[0][1]=0x20;CHECK(ep_poll(&p)==ED_LINK_DOWN && ep_validate(&p,&link)==ED_STALE);
 ep_note("OBS phy_mapped 1073905680 1073905684 100 8");return 0;
}
