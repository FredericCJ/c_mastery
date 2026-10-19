#include "firmware.h"
#include "frame.h"
#include <limits.h>
#define C(x) do{if(!(x))return __LINE__;}while(0)
static int ready(void *p){unsigned *n=p;return ++*n==3;}
static int boot(CfFirmware *f){unsigned n=0;return cf_init(f)&&cf_boot(f,ready,&n,3)==CF_OK;}
typedef struct {unsigned char bytes[256];size_t used;} Capture;
static CsIO capture(void *p,const unsigned char *b,size_t n){Capture *c=p;if(n>256-c->used)return(CsIO){0,CS_IO_ERROR,1};for(size_t i=0;i<n;++i)c->bytes[c->used++]=b[i];return(CsIO){n,CS_IO_PROGRESS,0};}
static int drain(CfFirmware *f,Capture *c){while(f->core.count){if(cs_begin(&f->core)!=CS_OK||cs_step(&f->core,capture,c)!=CS_DONE)return 0;}return 1;}
static int same(const unsigned char *a,const unsigned char *b,size_t n){for(size_t i=0;i<n;++i)if(a[i]!=b[i])return 0;return 1;}
static int service(void){CfFirmware poll,irq;C(boot(&poll)&&boot(&irq));Capture a={0},b={0};
    for(unsigned i=0;i<6;++i){CPFSample s={100+i,(uint16_t)(0xa500u+i),(uint16_t)(65530u+i)};C(cf_acquire(&poll,1,&s)==CF_OK&&cf_foreground(&poll,1)==CF_OK&&drain(&poll,&a));C(cf_acquire(&irq,1,&s)==CF_OK);if(i%3==2){for(unsigned j=0;j<3;++j)C(cf_foreground(&irq,1)==CF_OK);C(drain(&irq,&b));}}
    C(a.used==108&&a.used==b.used&&same(a.bytes,b.bytes,a.used));C(cf_acquire(&poll,0,0)==CF_PENDING&&cf_foreground(&poll,1)==CF_PENDING&&poll.feeds==0);
    for(unsigned i=0;i<4;++i){CPFSample s={200+i,1,1};C(cf_acquire(&irq,1,&s)==(i<3?CF_OK:CF_FULL));}C(irq.samples.dropped==1);
    for(unsigned i=0;i<3;++i)C(cf_foreground(&irq,1)==CF_OK);
    CPFSample s={205,1,1};C(cf_acquire(&irq,1,&s)==CF_OK&&cf_foreground(&irq,1)==CF_FULL&&irq.core.rejected==1);
    CfFirmware denied;C(boot(&denied));CPMessage m={1,1,{1}};C(cf_command(&denied,&m,0)==CF_DENIED&&denied.actuator==0&&!denied.core.accepted);C(cf_command(&denied,&m,1)==CF_OK&&denied.actuator==1);C(cf_command(&denied,&m,1)==CF_REPLAY&&denied.core.accepted==1);m.payload[0]=0;C(cf_command(&denied,&m,1)==CF_DENIED&&denied.actuator==1);m.sequence=2;C(cf_command(&denied,&m,1)==CF_OK&&denied.actuator==0);cf_fault(&denied,9);C(cf_command(&denied,&m,1)==CF_INVALID&&denied.actuator==0);
    C(cf_init(&denied));unsigned clock_reads=0;C(cf_boot(&denied,ready,&clock_reads,2)==CF_FAILED&&clock_reads==2&&denied.state==CF_SAFE_STOP&&denied.first_fault==1&&!denied.actuator);return 0;
}
typedef struct {CfFirmware *f;PdSnapshot s;unsigned stops,release,acquire;} Device;
static int configure(void *p,const PdExtent *e,uint32_t token){Device *d=p;if(d->s.enabled||e->source!=0x20001000u||e->destination!=0x20002000u||e->bytes!=8)return 0;d->s=(PdSnapshot){token,0,8,0,1};return 1;}
static void enable(void *p){((Device *)p)->s.enabled=1;}
static void stop(void *p){++((Device *)p)->stops;}
static PdSnapshot snapshot(void *p){return((Device *)p)->s;}
static void visibility(void *p,unsigned op,uint32_t address,uint32_t n){Device *d=p;(void)address;(void)n;if(op==PD_RELEASE)++d->release;if(op==PD_ACQUIRE)++d->acquire;}
static void write_bytes(Device *d,unsigned begin,unsigned end){for(unsigned i=begin;i<end;++i)d->f->dma_destination[i]=d->f->dma_source[i];d->s.remaining=8-end;}
static int dma(void){CfFirmware f;C(boot(&f));Device d={.f=&f};PdIo io={&d,configure,enable,stop,snapshot,visibility};const unsigned char sample[8]={7,0,0,0,0xa5,0,0xfe,0xff};for(unsigned i=0;i<8;++i)f.dma_source[i]=sample[i];
    C(cf_dma_begin(&f,io,0x20001000u,0x20002000u,1)==CF_OK&&f.dma_token==1);write_bytes(&d,0,4);C(cf_dma_cancel(&f,io,1)==CF_PENDING&&f.dma_token==1&&d.stops==1);cf_fault(&f,11);C(cf_recover(&f)==CF_PENDING&&cf_dma_finish(&f,io,1)==CF_PENDING);
    d.s.enabled=0;d.s.fifo_empty=0;write_bytes(&d,4,7);C(cf_dma_finish(&f,io,1)==CF_PENDING&&f.dma_token==1&&f.dma_destination[6]==0xfe);write_bytes(&d,7,8);d.s.fifo_empty=1;d.s.flags=PD_TC;C(cf_dma_finish(&f,io,1)==CF_FAILED&&!f.dma_token&&cf_foreground(&f,1)==CF_INVALID);C(cf_recover(&f)==CF_OK&&cf_foreground(&f,1)==CF_PENDING);
    C(cf_dma_begin(&f,io,0x20001000u,0x20002000u,2)==CF_OK);
    d.s.token=1;C(cf_dma_finish(&f,io,2)==CF_PENDING&&f.dma_token==2);
    d.s.token=2;write_bytes(&d,0,8);d.s.enabled=0;d.s.flags=PD_TC;
    C(cf_dma_finish(&f,io,2)==CF_OK&&!f.dma_token);
    C(cf_foreground(&f,1)==CF_OK&&f.core.accepted==1);
    Capture out={0};C(drain(&f,&out)&&out.used==18&&out.bytes[10]==0xa5);
    C(cf_dma_finish(&f,io,1)==CF_INVALID);return 0;
}
static int recovery(void){CfFirmware f;C(boot(&f));unsigned observations[]={1,3,2,3},expected[]={0,1,0,1};for(unsigned i=0;i<4;++i)C(cf_watchdog(&f,observations[i])==expected[i]);C(f.feeds==2);cf_fault(&f,21);C(f.state==CF_RECOVERING&&f.attempts==1&&f.first_fault==21&&cf_watchdog(&f,3)==0);C(cf_recover(&f)==CF_OK);cf_fault(&f,22);C(f.attempts==2&&f.first_fault==21&&cf_recover(&f)==CF_OK);cf_fault(&f,23);C(f.state==CF_SAFE_STOP&&f.first_fault==21&&f.actuator==0&&cf_recover(&f)==CF_INVALID);cf_fault(&f,24);C(f.attempts==2&&f.first_fault==21);
    CfCrash old,next;cf_crash(&old,7,21,4);for(unsigned cut=0;cut<=4;++cut){cf_crash(&next,8,22,cut);unsigned selected=9;C(cf_crash_select(&old,&next,&selected)&&selected==(cut==4));}cf_crash(&old,7,21,3);cf_crash(&next,8,22,0);unsigned selected=9;C(!cf_crash_select(&old,&next,&selected)&&selected==9);cf_crash(&old,UINT32_MAX,21,4);cf_crash(&next,1,22,4);C(cf_crash_select(&old,&next,&selected)&&selected==0);return 0;
}
static int update(void){CfSlot a,b;cf_candidate(&a,7,5,1,1,2);for(unsigned cut=0;cut<=5;++cut){cf_candidate(&b,8,cut,1,1,2);unsigned chosen=99;C(cf_select(&a,&b,&chosen)&&chosen==(cut==5));}for(unsigned parts=0;parts<2;++parts){cf_candidate(&b,8,5,1,1,parts);unsigned chosen=99;C(cf_select(&a,&b,&chosen)&&chosen==0);}cf_candidate(&b,8,5,1,0,2);unsigned chosen=99;C(cf_select(&a,&b,&chosen)&&chosen==0);cf_candidate(&b,8,5,0,1,2);C(cf_select(&a,&b,&chosen)&&chosen==0);cf_candidate(&b,8,5,1,1,2);b.payload[0]^=1;C(cf_select(&a,&b,&chosen)&&chosen==0);
    cf_candidate(&a,65535,5,1,1,2);cf_candidate(&b,0,5,1,1,2);C(cf_select(&a,&b,&chosen)&&chosen==1);cf_candidate(&b,32767,5,1,1,2);chosen=99;C(!cf_select(&a,&b,&chosen)&&chosen==99);return 0;
}
static int budget(void){uint32_t total=9,remaining=9;C(cf_budget(4096,0,&total,&remaining)&&total==1896&&remaining==2200);C(cf_budget(2048,0,&total,&remaining)&&remaining==152);total=remaining=9;C(!cf_budget(2048,160,&total,&remaining)&&total==9&&remaining==9);C(!cf_budget(UINT32_MAX,UINT32_MAX,&total,&remaining));
    uint32_t words[27]={0};words[6]=0x100;words[7]=1u<<24;XfFrame frame;C(xf_frame(words,8,0x20001000,0xfffffff9,1,0,1,0,&frame)==XF_OK&&frame.hardware_words==8);C(xf_frame(words,26,0x20001000,0xffffffe9,1,0,1,0,&frame)==XF_OK&&frame.hardware_words==26&&!frame.fp_materialized);C(cf_budget(4096,72,&total,&remaining)&&total==1968);return 0;
}
static int isolation(void){unsigned char request[8]={1,2,3,4,5,6,7,8},copied[8]={0};C(cf_copy_request(request,8,0,8,copied));request[0]=99;C(copied[0]==1);C(!cf_copy_request(request,8,SIZE_MAX,1,copied)&&copied[0]==1);C(!cf_copy_request(request,8,7,2,copied));unsigned synthetic=0;unsigned *ordinary=&synthetic;*ordinary=17;C(synthetic==17);/* C modules cannot deny this live-pointer write. */return 0;}
static int timing(void){uint16_t elapsed=99;C(cpf_elapsed16(65530,6,&elapsed)&&elapsed==12);uint32_t ticks=0;C(cf_elapsed(65530,6,1,&ticks)&&ticks==12);C(cf_elapsed(65530,6,2,&ticks)&&ticks==65548);C(!cf_elapsed(65530,6,0,&ticks));C(12u*1000000u/16000000u==0);/* Integer microsecond rounding loses this sub-microsecond interval. */CPFClock clock;uint64_t stamp=0;C(cpf_clock_init(&clock)&&cpf_clock_write(&clock,UINT64_C(0x100000006))&&cpf_clock_read(&clock,4,&stamp)&&stamp==UINT64_C(0x100000006));return 0;}
static int registers(void){CfRegister r={7};C(cf_status_ack(&r,1)&&r.flags==6);C(cf_status_read(&r)==6&&r.flags==2);C(cf_status_read(&r)==2&&r.flags==2);r.flags=7;unsigned seen=cf_status_read(&r);cf_status_write(&r,seen|1u);C(r.flags==0);/* Original RMW loses both unrelated W1C2 and read-clear4. */C(!cf_status_ack(&r,4));return 0;}
int cf_checks(unsigned which){switch(which){case 0:return service();case 1:return dma();case 2:return recovery();case 3:return update();case 4:return budget();case 5:return isolation();case 6:return timing();case 7:return registers();default:return -1;}}
