#include "bus.h"
#include "wrappers.h"
#ifdef MB_EXAMPLE_MAP
#include "linked_map_example.h"
#else
#include "linked_map.h"
#endif
#define CHECK(x) do{if(!(x)){mb_failure(#x,__LINE__);return 41;}}while(0)
extern struct aw_record ab_cpu_only[2],ab_dma_candidate[2];extern unsigned char mb_dma_source[16];
static int name(const char*a,const char*b){while(*a && *a==*b){++a;++b;}return *a==*b;}
static int equal(const void*a,const void*b,size_t n){const unsigned char*x=a,*y=b;for(size_t i=0;i<n;++i)if(x[i]!=y[i])return 0;return 1;}
static int topology(void){
 struct mb_map m,old;for(unsigned part=0;part<2;++part){CHECK(mb_map(part,21,&m)==MB_OK);CHECK(m.regions[MB_CCM].masters==(1u<<MB_CPU_D));CHECK(m.regions[MB_SRAM3].present==(part==MB_F439));CHECK(m.ordinary_bytes==(part==MB_F439?196608u:131072u));for(unsigned region=0;region<6;++region){struct mb_region r=m.regions[region];uint32_t row[]={part,region,r.base,r.bytes,r.present,r.masters,m.ordinary_bytes};mb_emit("map",row,7);}}
 old=m;CHECK(mb_map(2,21,&m)==MB_INVALID && equal(&m,&old,sizeof(m)));CHECK(mb_map(0,20,&m)==MB_INVALID && equal(&m,&old,sizeof(m)));return 0;
}
static int placement(void){
 uint32_t offset=999;CHECK(mb_place(0,21,MB_CCM,MB_DMA2_M,0x10000010,32,1,&offset)==MB_UNREACHABLE && offset==999);CHECK(mb_place(0,21,MB_SRAM1,MB_DMA2_M,0x2001bff0,32,1,&offset)==MB_EXTENT && offset==999);CHECK(mb_place(0,21,MB_SRAM3,MB_DMA2_M,0x20020010,64,4,&offset)==MB_OK && offset==16);CHECK(mb_place(0,21,MB_SRAM3,MB_DMA2_M,0x2002fff0,32,1,&offset)==MB_EXTENT && offset==16);
 const uint32_t sizes[]={1,16,32,UINT32_MAX};
 for(unsigned part=0;part<2;++part){struct mb_map map;CHECK(mb_map(part,21,&map)==MB_OK);for(unsigned reg=0;reg<4;++reg){struct mb_region r=map.regions[reg];uint32_t addresses[]={r.base,r.base+1,r.base+r.bytes-1,r.base+r.bytes,UINT32_MAX};for(unsigned master=0;master<6;++master)for(unsigned a=0;a<5;++a)for(unsigned n=0;n<4;++n){offset=0xfeed;int result=mb_place(part,21,reg,master,addresses[a],sizes[n],1,&offset);uint32_t row[]={part,reg,master,addresses[a],sizes[n],(uint32_t)result,offset};mb_emit("extent",row,7);}}}
 CHECK(mb_place(0,21,MB_SRAM1,MB_DMA2_P,MB_LINK_SOURCE,16,4,&offset)==MB_OK);CHECK(mb_place(0,21,MB_SRAM3,MB_DMA2_M,MB_LINK_DESTINATION,16,4,&offset)==MB_OK);CHECK(MB_LINK_SOURCE==MB_CCM_LINK_SOURCE && MB_LINK_DESTINATION==MB_CCM_LINK_DESTINATION);CHECK(mb_place(0,21,MB_CCM,MB_CPU_D,MB_LINK_CPU_CCM,16,4,&offset)==MB_OK);CHECK(mb_place(0,21,MB_CCM,MB_DMA2_M,MB_LINK_CPU_CCM,16,4,&offset)==MB_UNREACHABLE);CHECK(mb_place(0,21,MB_SRAM1,MB_CPU_S,MB_LINK_CPU_SRAM,16,4,&offset)==MB_OK);uint32_t row[]={MB_LINK_SOURCE,MB_LINK_DESTINATION,MB_LINK_CPU_SRAM,MB_LINK_CPU_CCM};mb_emit("linked",row,4);return 0;
}
struct mb_mock {struct pd_snapshot snapshot;unsigned configures,enables,stops,release,acquire;};
static int configure(void*p,const struct pd_extent*e,uint32_t token){struct mb_mock*m=p;++m->configures;m->snapshot=(struct pd_snapshot){token,0,e->bytes,0,1};return 1;}
static void enable(void*p){struct mb_mock*m=p;++m->enables;m->snapshot.enabled=1;}
static void stop(void*p){struct mb_mock*m=p;++m->stops;m->snapshot.enabled=0;}
static struct pd_snapshot snapshot(void*p){return ((struct mb_mock*)p)->snapshot;}
static void visibility(void*p,unsigned action,uint32_t address,uint32_t bytes){struct mb_mock*m=p;(void)address;(void)bytes;if(action==PD_RELEASE)++m->release;if(action==PD_ACQUIRE)++m->acquire;}
static void emit_lease(unsigned event,const struct mb_lease*s,const struct mb_mock*m){uint32_t row[]={event,s->transfer.phase,s->transfer.token,m->snapshot.token,m->snapshot.enabled,m->snapshot.remaining,m->snapshot.flags,m->snapshot.fifo_empty,s->source[0],s->destination[0],m->release,m->acquire};mb_emit("lease_state",row,12);}
static int lease(void){
 struct mb_lease s={0};struct mb_mock m={0};struct pd_io io={&m,configure,enable,stop,snapshot,visibility};unsigned char*destination=(unsigned char*)ab_dma_candidate;for(unsigned i=0;i<16;++i){mb_dma_source[i]=(unsigned char)(i+1);destination[i]=0;}CHECK(mb_bind(&s,mb_dma_source,destination,16,MB_LINK_SOURCE,MB_SRAM1,MB_LINK_DESTINATION,MB_SRAM3)==PD_OK);CHECK(mb_cpu_write(&s,0,42)==PD_OK);emit_lease(0,&s,&m);CHECK(mb_give(&s,io,1)==PD_OK && m.enables==1 && m.release==1);CHECK(mb_cpu_write(&s,0,99)==PD_BUSY && mb_dma_source[0]==42);unsigned char value=99;CHECK(mb_cpu_read(&s,0,&value)==PD_BUSY && value==99);struct pd_result result={99,99,99,99},old=result;CHECK(mb_finish(&s,io,1,&result)==PD_BUSY && equal(&result,&old,sizeof(result)));emit_lease(1,&s,&m);
 m.snapshot.enabled=0;m.snapshot.remaining=0;m.snapshot.flags=PD_TC;m.snapshot.fifo_empty=0;CHECK(mb_poll(&s,io,1)==PD_BUSY && s.transfer.phase!=3);CHECK(mb_cpu_write(&s,0,99)==PD_BUSY && mb_dma_source[0]==42);emit_lease(2,&s,&m);m.snapshot.fifo_empty=1;m.snapshot.token=2;CHECK(mb_poll(&s,io,1)==PD_BUSY);emit_lease(3,&s,&m);m.snapshot.token=1;for(unsigned i=0;i<16;++i)destination[i]=mb_dma_source[i];CHECK(mb_poll(&s,io,1)==PD_OK && s.transfer.phase==3);CHECK(mb_cpu_write(&s,0,99)==PD_BUSY && mb_dma_source[0]==42);CHECK(mb_cpu_read(&s,0,&value)==PD_BUSY && value==99);emit_lease(4,&s,&m);CHECK(mb_finish(&s,io,1,&result)==PD_OK && result.status==PD_OK && result.complete_bytes==16);CHECK(mb_cpu_read(&s,0,&value)==PD_OK && value==42);CHECK(mb_cpu_write(&s,0,77)==PD_OK);CHECK(mb_finish(&s,io,1,&result)==PD_STALE);CHECK(mb_give(&s,io,1)==PD_STALE);emit_lease(5,&s,&m);uint32_t row[]={m.configures,m.enables,m.release,m.acquire,result.complete_bytes,value,mb_dma_source[0]};mb_emit("lease",row,7);return 0;
}
static int arbitration(void){
 struct mb_bus_trace r,old;struct mb_bus_config c={7,{1,1,1},{0,0,0},2,1,0};CHECK(mb_arbitrate(c,&r)==MB_OK && r.count==6);for(unsigned i=0;i<6;++i)CHECK(r.master[i]==i%3 && r.start[i]==i && r.finish[i]==i+1);
 for(unsigned mask=1;mask<=7;++mask)for(unsigned mode=0;mode<2;++mode){c=(struct mb_bus_config){mask,{2,4,1},{0,1,2},3,4,2};if(!mode)c=(struct mb_bus_config){mask,{1,1,1},{0,0,0},3,1,0};CHECK(mb_arbitrate(c,&r)==MB_OK);if(mode && mask==7)CHECK(r.finish[2]==13 && r.wait[3]==11);for(unsigned i=0;i<r.count;++i){CHECK(r.wait[i]<=r.bound);uint32_t row[]={mask,mode,i,r.master[i],r.start[i],r.finish[i],r.wait[i],r.bound};mb_emit("grant",row,8);}}
 old=r;c.max_beats=0;CHECK(mb_arbitrate(c,&r)==MB_INVALID && equal(&r,&old,sizeof(r)));return 0;
}
static int bridge(void){
 uint32_t result=0;CHECK(mb_bridge(8,0x12,&result)==MB_OK && result==0x12121212);CHECK(mb_bridge(16,0x1234,&result)==MB_OK && result==0x12341234);CHECK(!mb_usart_width(8) && mb_usart_width(16) && mb_usart_width(32));
 for(unsigned w=0;w<3;++w){unsigned width=8u<<w;for(unsigned i=0;i<5;++i){uint32_t value=(uint32_t[]){0,1,0x12,width==8?255u:65535u,width==8?256u:width==16?65536u:UINT32_MAX}[i];result=0xfeed;int status=mb_bridge(width,value,&result);uint32_t row[]={width,value,(uint32_t)status,result,(uint32_t)mb_usart_width(width)};mb_emit("bridge",row,5);}}
 return 0;
}
int mb_test(const char*s){if(name(s,"topology"))return topology();if(name(s,"placement"))return placement();if(name(s,"lease"))return lease();if(name(s,"arbitration"))return arbitration();if(name(s,"bridge"))return bridge();return 42;}
