#include "progress.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ER_CHECK EPT_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
struct er_sink {size_t next;er_code code;unsigned calls;unsigned char bytes[128];size_t used,offered;};
static er_progress sink(void *context,const unsigned char *bytes,size_t n){struct er_sink *s=context;++s->calls;s->offered=n;if(s->next<=n&&s->used+s->next<=128){for(size_t i=0;i<s->next;++i)s->bytes[s->used++]=bytes[i];}er_progress r={s->next,s->code};return r;}
struct er_flush {struct ept_flush_result result;unsigned calls;size_t accepted,prior;uint32_t epoch;};
static struct ept_flush_result flush(void *context,size_t accepted,size_t prior,uint32_t epoch){struct er_flush *f=context;++f->calls;f->accepted=accepted;f->prior=prior;f->epoch=epoch;return f->result;}
static int core(void){
    struct ept_stream s={0};struct ept_handle h={0};struct ept_observation o;unsigned char payload[]={1,2,3};ept_init(&s);CHECK(ept_start(&s,payload,3,&h)==EPT_OK);payload[0]=99;
    struct er_sink b={.next=2,.code=ER_AGAIN};CHECK(ept_step(h,sink,&b,&o)==EPT_AGAIN&&o.accepted==2&&o.durable==0&&b.calls==1&&b.bytes[0]==3&&b.bytes[1]==1);
    b.next=3;b.code=ER_IO;CHECK(ept_step(h,sink,&b,&o)==EPT_IO&&o.accepted==5&&o.durable==0&&b.calls==2&&b.bytes[4]==6);
    CHECK(ept_step(h,sink,&b,&o)==EPT_IO&&b.calls==2);CHECK(ept_release(h,1,1)==EPT_OK);
    CHECK(ept_observe(h,&o)==EPT_STALE);CHECK(ept_start(&s,NULL,0,&h)==EPT_OK);b=(struct er_sink){.next=2,.code=ER_OK};CHECK(ept_step(h,sink,&b,&o)==EPT_DONE&&o.accepted==2&&b.bytes[0]==0&&b.bytes[1]==0);CHECK(ept_release(h,1,1)==EPT_OK);
    puts("EPT_CORE actual-sf-copy actual-er-transfer status-with-final-progress separate-durability");return 0;
}
static int cancel(void){
    struct ept_stream s={0};struct ept_handle h={0},old;struct ept_observation o;unsigned char payload[]={1,2,3};ept_init(&s);CHECK(ept_start(&s,payload,3,&h)==EPT_OK);old=h;struct er_sink b={.next=2,.code=ER_AGAIN};CHECK(ept_step(h,sink,&b,&o)==EPT_AGAIN);CHECK(ept_cancel(h,&o)==EPT_CANCELLED&&o.accepted==2&&o.durable==0);CHECK(ept_step(h,sink,&b,&o)==EPT_CANCELLED&&b.calls==1);
    CHECK(ept_start(&s,payload,3,&h)==EPT_BUSY);CHECK(ept_release(h,0,1)==EPT_BUSY);CHECK(ept_release(h,1,0)==EPT_BUSY);CHECK(ept_release(h,1,1)==EPT_OK);CHECK(ept_start(&s,payload,3,&h)==EPT_OK&&h.generation!=old.generation);CHECK(ept_cancel(old,&o)==EPT_STALE);CHECK(ept_cancel(h,&o)==EPT_CANCELLED);CHECK(ept_release(h,1,0)==EPT_OK);
    s.frame.generation=UINT32_MAX;old=h;CHECK(ept_start(&s,payload,3,&h)==EPT_RANGE&&h.generation==old.generation);
    puts("EPT_CANCEL copied-owner retained-prefix no-retry explicit-abandon generation-retirement");return 0;
}
static int epochs(void){
    struct ept_stream s={0};struct ept_handle h={0};struct ept_observation o={0};ept_init(&s);CHECK(ept_start(&s,(const unsigned char *)"abc",3,&h)==EPT_OK);struct er_sink b={.next=4,.code=ER_AGAIN};CHECK(ept_step(h,sink,&b,&o)==EPT_AGAIN&&o.accepted==4);
    struct er_flush f={.result={2,1,EPT_FLUSH_IO}};CHECK(ept_checkpoint(h,flush,&f,&o)==EPT_IO&&o.durable==2&&o.epoch==1&&f.accepted==4&&f.prior==0);
    f.result=(struct ept_flush_result){999,99,EPT_FLUSH_UNKNOWN};CHECK(ept_checkpoint(h,flush,&f,&o)==EPT_UNCERTAIN&&o.durable==2&&o.epoch==1&&o.uncertain==1);CHECK(ept_step(h,sink,&b,&o)==EPT_UNCERTAIN&&b.calls==1);CHECK(ept_checkpoint(h,flush,&f,&o)==EPT_UNCERTAIN&&f.calls==2);
    CHECK(ept_reconcile(h,4,2,0,&o)==EPT_BUSY);CHECK(ept_reconcile(h,5,2,1,&o)==EPT_RANGE);CHECK(ept_reconcile(h,4,2,1,&o)==EPT_OK&&o.durable==4&&o.epoch==2&&!o.uncertain);
    b.next=1;b.code=ER_OK;CHECK(ept_step(h,sink,&b,&o)==EPT_DONE&&o.accepted==5);f.result=(struct ept_flush_result){5,3,EPT_FLUSH_OK};CHECK(ept_checkpoint(h,flush,&f,&o)==EPT_OK&&o.durable==5&&o.epoch==3);CHECK(ept_checkpoint(h,flush,&f,&o)==EPT_OK&&f.calls==3);CHECK(ept_release(h,1,0)==EPT_OK);
    CHECK(ept_start(&s,(const unsigned char *)"abc",3,&h)==EPT_OK);b=(struct er_sink){.next=1,.code=ER_AGAIN};CHECK(ept_step(h,sink,&b,&o)==EPT_AGAIN);s.epoch=UINT32_MAX;CHECK(ept_checkpoint(h,flush,&f,&o)==EPT_RANGE&&f.calls==3);CHECK(ept_cancel(h,&o)==EPT_CANCELLED);CHECK(ept_release(h,1,1)==EPT_OK);
    puts("EPT_EPOCHS accepted-vs-durable error-with-commit lost-ack trusted-reconcile finite-epoch");return 0;
}
static int uncertain(void){
    struct ept_stream s={0};struct ept_handle h={0};struct ept_observation o;ept_init(&s);CHECK(ept_start(&s,(const unsigned char *)"abc",3,&h)==EPT_OK);struct er_sink b={.next=6,.code=ER_OK};CHECK(ept_step(h,sink,&b,&o)==EPT_CONTRACT&&o.accepted==0&&o.uncertain==2);CHECK(ept_step(h,sink,&b,&o)==EPT_UNCERTAIN&&b.calls==1);CHECK(ept_reconcile(h,0,0,1,&o)==EPT_BUSY);CHECK(ept_release(h,1,0)==EPT_BUSY);CHECK(ept_release(h,1,1)==EPT_OK);
    CHECK(ept_start(&s,(const unsigned char *)"abc",3,&h)==EPT_OK);b=(struct er_sink){.next=2,.code=ER_AGAIN};CHECK(ept_step(h,sink,&b,&o)==EPT_AGAIN);struct er_flush f={.result={3,1,EPT_FLUSH_OK}};CHECK(ept_checkpoint(h,flush,&f,&o)==EPT_CONTRACT&&o.durable==0&&o.uncertain==1);CHECK(ept_cancel(h,&o)==EPT_CANCELLED);CHECK(ept_release(h,1,1)==EPT_OK);
    puts("EPT_UNCERTAIN invalid-report retains-lower-bound no-automatic-replay no-readability-proof");return 0;
}
static int base(void){unsigned char bytes[4]={1,2,3,4};struct er_sink b={.next=2,.code=ER_IO};er_progress p=er_transfer(bytes,4,0,sink,&b);CHECK(p.next==2&&p.code==ER_IO&&b.calls==1);b.next=0;b.code=ER_AGAIN;p=er_transfer(bytes,4,2,sink,&b);CHECK(p.next==2&&p.code==ER_AGAIN&&b.offered==2);puts("EPT_BASE actual-er-transfer error-progress status-retained");return 0;}
static int oracle(void){unsigned mode,code,epoch;size_t length,next,count;unsigned char bytes[66]={0};while(scanf("%u %zu %zu %zu %u %u",&mode,&length,&next,&count,&code,&epoch)==6){CHECK(length<=64);struct er_sink b={.next=count,.code=(er_code)code};
    if(mode==0){er_progress p=er_transfer(bytes,length,next,sink,&b);printf("%u %zu %u %zu\n",(unsigned)p.code,p.next,b.calls,b.offered);}
    else{struct ept_stream s={0};struct ept_handle h={0};struct ept_observation o;ept_init(&s);CHECK(ept_start(&s,bytes,length,&h)==EPT_OK);
        if(mode==1){unsigned result=ept_step(h,sink,&b,&o);printf("%u %zu %zu %u %u %zu\n",result,o.accepted,o.durable,o.uncertain,b.calls,b.offered);}
        else{b.next=next;b.code=ER_AGAIN;CHECK(ept_step(h,sink,&b,&o)==EPT_AGAIN);struct er_flush f={.result={count,epoch,code}};unsigned result=ept_checkpoint(h,flush,&f,&o);printf("%u %zu %zu %" PRIu32 " %u %u\n",result,o.accepted,o.durable,o.epoch,o.uncertain,f.calls);}
    }
}return 0;}
static int metrics(void){double seconds[7];uint64_t checksum=0;for(unsigned k=0;k<7;++k){clock_t start=clock();for(unsigned i=0;i<10000;++i){struct ept_stream s={0};struct ept_handle h={0};struct ept_observation o;ept_init(&s);CHECK(ept_start(&s,(const unsigned char *)"abc",3,&h)==EPT_OK);struct er_sink b={.next=5,.code=ER_OK};CHECK(ept_step(h,sink,&b,&o)==EPT_DONE);struct er_flush f={.result={5,1,EPT_FLUSH_OK}};CHECK(ept_checkpoint(h,flush,&f,&o)==EPT_OK);checksum+=o.durable;CHECK(ept_release(h,1,0)==EPT_OK);}seconds[k]=(double)(clock()-start)/CLOCKS_PER_SEC;}printf("{\"iterations\":10000,\"checksum\":%" PRIu64 ",\"stream_size\":%zu,\"stream_align\":%zu,\"copied_frame_bytes\":5,\"allocation_calls\":0,\"seconds\":[",checksum,sizeof(struct ept_stream),_Alignof(struct ept_stream));for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",seconds[i]);puts("]}");return 0;}
int main(int n,char **v){if(n!=2)return 2;if(!strcmp(v[1],"progress_core"))return core();if(!strcmp(v[1],"progress_cancel"))return cancel();if(!strcmp(v[1],"progress_epochs"))return epochs();if(!strcmp(v[1],"progress_uncertain"))return uncertain();if(!strcmp(v[1],"progress_base"))return base();if(!strcmp(v[1],"oracle"))return oracle();if(!strcmp(v[1],"metrics"))return metrics();return 2;}
