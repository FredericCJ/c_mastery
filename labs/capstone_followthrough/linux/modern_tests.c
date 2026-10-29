#define _POSIX_C_SOURCE 200809L
#include "modern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CAP_MODERN:%s:%d\n",#x,__LINE__);return 1;}}while(0)
struct cf_script { size_t call,total;unsigned char bytes[13];int bad,error; };
static struct cs_io scripted(void *p,const unsigned char *b,size_t n){struct cf_script *s=p;static const size_t counts[]={2,0,0,0,3,8};size_t k=counts[s->call];
    if(s->bad)return(struct cs_io){n+1,CS_IO_PROGRESS,0};
    if(s->error && s->call==5){++s->call;return(struct cs_io){0,CS_IO_ERROR,5};}
    if(k>n || s->total+k>13)abort();
    for(size_t i=0;i<k;++i)s->bytes[s->total++]=b[i];
    enum cs_io_kind kind=s->call==1?CS_IO_AGAIN:s->call==3?CS_IO_INTERRUPTED:CS_IO_PROGRESS;++s->call;return(struct cs_io){k,kind,0};}
static uint32_t success_fn(uint32_t v,void *p){++*(unsigned *)p;return v;}
static int result(void){const unsigned char bytes[13]={0xa5,1,2,1,3,0,0,0x41,0xff,0x62,0xf6,0x8b,0x76};
    struct cf_script s={0};struct cs_result_sink a={scripted,&s};er_progress p={0,ER_AGAIN};const size_t cursors[]={2,2,2,2,5,13};
    for(unsigned i=0;i<6;++i){p=cs_result_step(bytes,13,p.next,&a);CHECK(p.next==cursors[i] && p.code==(i==1||i==2||i==3?ER_AGAIN:ER_OK));}
    CHECK(s.total==13 && !memcmp(s.bytes,bytes,13));s=(struct cf_script){.bad=1};p=cs_result_step(bytes,13,5,&a);CHECK(p.next==5 && p.code==ER_CONTRACT);
    s=(struct cf_script){.error=1};p=(er_progress){0,ER_AGAIN};for(unsigned i=0;i<6;++i)p=cs_result_step(bytes,13,p.next,&a);CHECK(p.next==5 && p.code==ER_IO);
    unsigned calls=0;er_result r=er_map(er_error(p.code,p.next),success_fn,&calls);CHECK(r.tag==ER_IO && r.as.offset==5 && calls==0);
    printf("result-size=%zu progress-size=%zu adapter-size=%zu callbacks=6 accepted=5 error=retained\n",sizeof(er_result),sizeof(er_progress),sizeof(struct cs_result_sink));return 0;}
static int slabs(void){struct cs_slabs p;cs_slabs_init(&p);struct cp_message m={1,1,{0xa5}};struct cs_slab_token tokens[4][2];
    for(unsigned i=0;i<4;++i){m.sequence=(uint16_t)i;CHECK(cs_slabs_share(&p,&m,tokens[i])==CS_OK && p.slot[i].rights.strong==2);}
    struct cs_slab_token unchanged[2]={{99,99,99},{99,99,99}};CHECK(cs_slabs_share(&p,&m,unchanged)==CS_FULL && unchanged[0].slot==99);
    const unsigned char *view;size_t n;CHECK(cs_slabs_view(&p,tokens[0][1],&view,&n)==CS_OK && n==11);CHECK(cs_slabs_drop(&p,tokens[0][0])==CS_OK);
    CHECK(p.slot[0].destructions==0 && view[0]==0xa5 && p.slot[0].rights.strong==1);CHECK(cs_slabs_drop(&p,tokens[0][0])==CS_STALE);
    CHECK(cs_slabs_drop(&p,tokens[0][1])==CS_OK && p.slot[0].destructions==1);CHECK(cs_slabs_view(&p,tokens[0][1],&view,&n)==CS_STALE);
    for(unsigned i=1;i<4;++i){CHECK(cs_slabs_drop(&p,tokens[i][1])==CS_OK);CHECK(cs_slabs_drop(&p,tokens[i][0])==CS_OK);}
    for(unsigned generation=2;generation<=3;++generation){CHECK(cs_slabs_share(&p,&m,tokens[0])==CS_OK && tokens[0][0].generation==generation);CHECK(cs_slabs_drop(&p,tokens[0][0])==CS_OK && cs_slabs_drop(&p,tokens[0][1])==CS_OK);}
    CHECK(p.slot[0].generation==4);CHECK(cs_slabs_share(&p,&m,tokens[0])==CS_OK && tokens[0][0].slot==1);
    CHECK(cs_slabs_drop(&p,tokens[0][0])==CS_OK && cs_slabs_drop(&p,tokens[0][1])==CS_OK);
    printf("slabs=4 consumers=2 backing=168 control-bytes=%zu last-drop=exactly-once\n",sizeof p-168);return 0;}
static int receipts(void){struct cs_receipts r={.epoch=7};struct cp_message m={19,3,{1,2,3}};CHECK(cs_receipt(&r,7,&m)==CS_FRAME && r.effects==1);
    CHECK(cs_receipt(&r,7,&m)==CS_DONE && r.effects==1);m.payload[1]=9;CHECK(cs_receipt(&r,7,&m)==CS_REJECT && r.effects==1);CHECK(cs_receipt(&r,8,&m)==CS_INVALID);
    for(unsigned i=0;i<3;++i){m.sequence=(uint16_t)(20+i);CHECK(cs_receipt(&r,7,&m)==CS_FRAME);}m.sequence=23;CHECK(cs_receipt(&r,7,&m)==CS_FULL && r.effects==4);
    puts("receipts=epoch7+sequence+exact-payload duplicate-effect=0 capacity4-no-eviction");return 0;}
struct cf_borrow {pthread_mutex_t mutex;pthread_cond_t changed;int waiting,go;unsigned char *bytes;int observed;};
static void *worker(void *p){struct cf_borrow *b=p;pthread_mutex_lock(&b->mutex);b->waiting=1;pthread_cond_signal(&b->changed);while(!b->go)pthread_cond_wait(&b->changed,&b->mutex);b->observed=b->bytes[0];pthread_mutex_unlock(&b->mutex);return NULL;}
static int cleanup(void){int input[2],output[2];CHECK(pipe(input)==0 && pipe(output)==0);int ep=epoll_create1(EPOLL_CLOEXEC);CHECK(ep>=0);
    for(unsigned fail=0;fail<=6;++fail){unsigned prior[5]={0};for(int modern=0;modern<2;++modern){struct cs_resources r;int result=cs_resources_open(&r,ep,input[0],output[1],fail,modern);CHECK((result==0)==(fail==0));
            if(!fail){CHECK(r.published && r.acquired==31);CHECK(write(r.output,"OK",2)==2);CHECK(cs_resources_close(&r)==0);}
            unsigned expected=fail?fail-1:5;CHECK(!r.published && !r.acquired && r.released==expected && !r.cleanup_error);
            for(unsigned i=0;i<expected;++i){CHECK(r.release_ids[i]==expected-i);if(!modern)prior[i]=r.release_ids[i];else CHECK(prior[i]==r.release_ids[i]);}
            CHECK(cs_resources_close(&r)==0 && r.released==expected);
        }}
    char accepted[4];CHECK(read(output[0],accepted,4)==4 && !memcmp(accepted,"OKOK",4));
    struct cs_resources r;CHECK(cs_resources_open(&r,ep,input[0],output[1],0,1)==0 && cs_resources_borrow(&r)==0);((unsigned char *)r.candidate)[0]=93;
    struct cf_borrow b={.mutex=PTHREAD_MUTEX_INITIALIZER,.changed=PTHREAD_COND_INITIALIZER,.bytes=r.candidate};pthread_t t;CHECK(!pthread_create(&t,NULL,worker,&b));
    CHECK(!pthread_mutex_lock(&b.mutex));while(!b.waiting)CHECK(!pthread_cond_wait(&b.changed,&b.mutex));CHECK(cs_resources_close(&r)==1 && r.released==0 && r.acquired==31 && r.candidate==b.bytes);
    b.go=1;CHECK(!pthread_cond_signal(&b.changed) && !pthread_mutex_unlock(&b.mutex));CHECK(!pthread_join(t,NULL) && b.observed==93);CHECK(cs_resources_ack(&r)==0 && r.released==5);
    CHECK(!pthread_cond_destroy(&b.changed) && !pthread_mutex_destroy(&b.mutex));CHECK(!close(ep) && !close(input[0]) && !close(input[1]) && !close(output[0]) && !close(output[1]));
    printf("cleanup-prefixes=7 policies=2 actual-fds+heap+epoll retained-output=OKOK scope-bytes=%zu ack-after-join=5\n",sizeof(struct ow_scope));return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;
    if(!strcmp(argv[1],"result"))return result();
    if(!strcmp(argv[1],"slabs"))return slabs();
    if(!strcmp(argv[1],"receipts"))return receipts();
    if(!strcmp(argv[1],"cleanup"))return cleanup();
    return 2;}
