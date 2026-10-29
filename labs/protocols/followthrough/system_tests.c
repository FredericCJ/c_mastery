#include "system_follow.h"
#include <stdio.h>
#include <stdlib.h>
#define C(x) do{if(!(x)){fprintf(stderr,"PS_CHECK:%s:%d\n",#x,__LINE__);exit(1);}}while(0)
struct pf_env{unsigned busy,feeds,stops,frees,writes[FW_WATCHDOG+1];void *owned;};
static uint32_t read_reg(void *p,unsigned r){struct pf_env *e=p;return r==FW_DMA?e->busy:FW_TXE;}
static void write_reg(void *p,unsigned r,uint32_t v){struct pf_env *e=p;C(r<=FW_WATCHDOG);++e->writes[r];if(r==FW_DMA){if(v)e->busy=1;else ++e->stops;}if(r==FW_WATCHDOG)++e->feeds;}
static void release(void *p,void *v){struct pf_env *e=p;C(e->owned==v);free(v);e->owned=0;++e->frees;}
static void deadline(void)
{
 struct pf_system s;struct pf_env e={0};struct fw_io io={&e,read_reg,write_reg};C(pf_system_init(&s,io,31,release,&e)==PT_OK);
 unsigned char*p=malloc(4);C(p);e.owned=p;struct pf_recovery r;C(pf_driver_start(&s.driver,&p,4,0,&r)==PT_OK);
 C(pf_system_arm_deadline(&s,r,UINT64_MAX,1)==PT_RANGE&&!s.timer_armed);
 C(pf_system_arm_deadline(&s,r,100,10)==PT_OK&&s.timer_armed);
 C(pf_system_tick(&s,109)==PT_PENDING&&!s.queue.count&&!e.stops&&!e.frees);
 C(pf_system_tick(&s,108)==PT_RANGE&&s.last_time==109);
 struct ad_brand b;for(unsigned i=0;i<4;i++)C(pf_system_post(&s,0,PF_COMPLETE,r,31,0,&b)==PT_OK);
 C(pf_system_tick(&s,110)==PT_RANGE&&s.timer_armed&&!e.frees);
 C(pf_system_step(&s)==PT_PENDING);C(pf_system_tick(&s,110)==PT_OK&&!s.timer_armed);
 for(unsigned i=0;i<4;i++)C(pf_system_step(&s)==PT_PENDING);
 C(e.stops==1&&!e.frees&&s.driver.owned&&s.driver.reset_pending);
 e.busy=0;C(pf_system_post(&s,1,PF_QUIET,r,31,1,&b)==PT_OK);C(pf_system_step(&s)==PT_OK&&e.frees==1);
 C(pf_system_tick(&s,111)==PT_OK&&!s.queue.count);
}
int main(void)
{
 struct pf_system s;struct pf_env e={0};struct fw_io io={&e,read_reg,write_reg};C(pf_system_init(&s,io,9,release,&e)==PT_OK);C(e.writes[FW_GPIO]==1&&e.writes[FW_DATA]==1&&e.writes[FW_PERIOD]==1&&e.writes[FW_COMPARE]==1);C(cmx_schedule(&s.kernel)==2&&cmx_invariant(&s.kernel));
 unsigned char *p=malloc(4);C(p);e.owned=p;struct pf_recovery r;C(pf_driver_start(&s.driver,&p,4,0,&r)==PT_OK&&!p);struct ad_brand brand;C(pf_system_post(&s,0,PF_COMPLETE,r,9,0,&brand)==PT_OK);e.busy=0;C(pf_system_step(&s)==PT_PENDING&&!e.frees&&!e.feeds);uint32_t value=99;C(!ad_pool_get(&s.pool,brand,&value)&&value==99);
 C(pf_system_post(&s,1,PF_RESET,r,9,0,&brand)==PT_OK);C(pf_system_step(&s)==PT_PENDING&&s.driver.owned);C(pf_system_post(&s,0,PF_QUIET,r,10,1,&brand)==PT_OK);C(pf_system_step(&s)==PT_STATE&&!e.frees);C(pf_system_post(&s,1,PF_QUIET,r,9,0,&brand)==PT_OK);C(pf_system_step(&s)==PT_PENDING&&!e.frees);C(pf_system_post(&s,1,PF_QUIET,r,9,1,&brand)==PT_OK);C(pf_system_step(&s)==PT_OK&&e.frees==1&&s.driver.epoch==10);C(!ad_pool_get(&s.pool,brand,&value));
 int32_t expected[]={PT_PENDING,PT_PENDING,PT_STATE,PT_PENDING,PT_OK};for(unsigned i=0;i<5;++i){int32_t result=-1;C(algo_ring_take(&s.history,&result)==ALGO_OK&&result==expected[i]);}C(pf_system_step(&s)==PT_PENDING);C(cmx_invariant(&s.kernel));
 /* Queued stale tokens consume only their message brands, never a new loan. */
 C(pf_driver_ready(&s.driver)==PT_OK);p=malloc(2);C(p);e.owned=p;struct pf_recovery next;C(pf_driver_start(&s.driver,&p,2,0,&next)==PT_OK);for(unsigned i=0;i<4;++i)C(pf_system_post(&s,i%2,PF_COMPLETE,r,9,1,&brand)==PT_OK);struct ad_brand canary={0};C(pf_system_post(&s,0,PF_COMPLETE,r,9,1,&canary)==PT_RANGE&&!canary.owner);for(unsigned i=0;i<4;++i)C(pf_system_step(&s)==PT_STATE&&e.frees==1);e.busy=0;C(pf_system_post(&s,0,PF_COMPLETE,next,10,1,&brand)==PT_OK);C(pf_system_step(&s)==PT_OK&&e.frees==2&&e.feeds==1);puts("PS_PASS:all nine producer artifacts consumed; epoch-qualified queued recovery");
 deadline();
}
