#include "pin_dma.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"OW_CHECK OPA_DMA line%d: %s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {unsigned char source[16],destination[16];PdSnapshot snapshot;OpaDma *owner;unsigned configures,enables,stops,visible,published,busy;} Model;
static int configure(void *p,const PdExtent *e,uint32_t token){Model *m=p;++m->configures;if(m->busy)return 0;if(e->source!=0x20000000u||e->destination!=0x20002000u||e->bytes!=16||e->width!=1)return 0;m->snapshot=(PdSnapshot){token,0,16,0,1};return 1;}
static void enable(void *p){Model *m=p;++m->enables;m->published=m->owner->owner.pin.pending;m->snapshot.enabled=1;m->snapshot.fifo_empty=0;}
static void stop(void *p){++((Model *)p)->stops;}
static PdSnapshot snapshot(void *p){return ((Model *)p)->snapshot;}
static void visibility(void *p,unsigned kind,uint32_t a,uint32_t n){Model *m=p;(void)kind;(void)a;(void)n;++m->visible;}
static void advance(Model *m,unsigned count){for(unsigned i=0;i<count;++i){unsigned at=16-m->snapshot.remaining;m->destination[at]=m->source[at];--m->snapshot.remaining;}}
int pin_dma_test(void){
 Model m={0};for(unsigned i=0;i<16;++i){m.source[i]=(unsigned char)(i+11);m.destination[i]=0xA5;}OpaDma d;PdIo io={&m,configure,enable,stop,snapshot,visibility};m.owner=&d;CHECK(opa_dma_init(&d,&m,io)==OW_OK);PdExtent e={0x20000000u,0x20002000u,16,16,16,1};uint32_t ticket=91;PdResult out={92,93,94,95};m.busy=1;CHECK(opa_dma_submit(&d,&e,1,PD_F439,0,&ticket)==OW_BUSY&&!d.owner.pin.pending&&ticket==91&&!m.enables&&!d.owner.async.generation);m.busy=0;CHECK(opa_dma_submit(&d,&e,1,PD_F439,0,&ticket)==OW_OK&&ticket==1&&m.published&&m.enables==1);CHECK(opa_dma_timeout(&d,ticket)==OW_OK&&m.stops==1&&d.owner.recovery&&d.owner.pin.pending);advance(&m,4);CHECK(opa_dma_poll(&d,ticket,&out)==OW_BUSY&&out.token==92&&d.owner.pin.pending&&!d.owner.async.releases);m.snapshot.enabled=0;m.snapshot.flags=PD_TC;advance(&m,4);CHECK(opa_dma_poll(&d,ticket,&out)==OW_BUSY&&d.owner.pin.pending);m.snapshot.fifo_empty=1;m.snapshot.flags=0;CHECK(opa_dma_poll(&d,ticket,&out)==OW_BUSY&&d.owner.pin.pending);m.snapshot.flags=PD_TC;m.snapshot.token=0;CHECK(opa_dma_poll(&d,ticket,&out)==OW_BUSY&&out.token==92);m.snapshot.token=ticket;CHECK(opa_dma_poll(&d,ticket,&out)==OW_OK&&out.token==1&&out.status==PD_CANCELLED&&out.complete_bytes==8&&out.possibly_written_bytes==8&&!d.owner.pin.pending&&d.owner.async.releases==1);for(unsigned i=0;i<16;++i)CHECK(m.destination[i]==(i<8?m.source[i]:0xA5));CHECK(opa_dma_poll(&d,ticket,&out)==OW_STALE&&out.complete_bytes==8);
 CHECK(opa_dma_submit(&d,&e,1,PD_F439,0,&ticket)==OW_OK&&ticket==2);advance(&m,4);m.snapshot.enabled=0;m.snapshot.fifo_empty=1;m.snapshot.flags=PD_TC|PD_TE;CHECK(opa_dma_poll(&d,ticket,&out)==OW_OK&&out.status==PD_IO&&out.complete_bytes==0&&out.possibly_written_bytes==16&&d.owner.async.releases==2);unsigned calls=m.configures;d.owner.async.releases=UINT32_MAX;CHECK(opa_dma_submit(&d,&e,1,PD_F439,0,&ticket)==OW_FULL&&m.configures==calls&&!d.owner.pin.pending);
 puts("OPA_DMA actual-Pd-FAsync-OwPin late-model-writes EN-FIFO-TC-generation possible-versus-complete");return 0;
}
#ifdef OPA_DMA_STANDALONE
int main(void){return pin_dma_test();}
#endif
