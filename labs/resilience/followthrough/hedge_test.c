#include "hedge.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do{if(!(c)){fprintf(stderr,"RZ_CHECK: %s:%d %s\n",__FILE__,__LINE__,#c);exit(1);}}while(0)
struct rz_context {
    pthread_mutex_t mutex;pthread_cond_t changed;atomic_uint_fast64_t time;
    unsigned decode_pause,decode_enter,decode_release,dispose_pause,dispose_enter,dispose_release,done;
    atomic_uint freed[2],scopes;
};
struct rz_worker {struct rzh_arbiter *arbiter;struct rzh_ticket ticket;struct rzh_blob blob;struct rz_context *context;unsigned bit;int status;};
static void context_init(struct rz_context *c) {
    *c=(struct rz_context){0};CHECK(!pthread_mutex_init(&c->mutex,NULL));CHECK(!pthread_cond_init(&c->changed,NULL));
    atomic_init(&c->time,0);atomic_init(&c->freed[0],0);atomic_init(&c->freed[1],0);atomic_init(&c->scopes,0);
}
static void context_destroy(struct rz_context *c){CHECK(!pthread_cond_destroy(&c->changed));CHECK(!pthread_mutex_destroy(&c->mutex));}
static uint64_t clock_read(void *p){return atomic_load(&((struct rz_context *)p)->time);}
static void wait_mask(struct rz_context *c,unsigned *field,unsigned mask) {
    CHECK(!pthread_mutex_lock(&c->mutex));while((*field&mask)!=mask)CHECK(!pthread_cond_wait(&c->changed,&c->mutex));CHECK(!pthread_mutex_unlock(&c->mutex));
}
static void release_mask(struct rz_context *c,unsigned *field,unsigned mask) {
    CHECK(!pthread_mutex_lock(&c->mutex));*field|=mask;CHECK(!pthread_cond_broadcast(&c->changed));CHECK(!pthread_mutex_unlock(&c->mutex));
}
static int decode(void *p,const unsigned char *data,size_t n,unsigned *value) {
    struct rz_context *c=p;if(!data || n<6 || data[0]>1)return RZ_INVALID;
    unsigned bit=1u<<data[0];CHECK(!pthread_mutex_lock(&c->mutex));c->decode_enter|=bit;CHECK(!pthread_cond_broadcast(&c->changed));
    while((c->decode_pause&bit) && !(c->decode_release&bit))CHECK(!pthread_cond_wait(&c->changed,&c->mutex));
    CHECK(!pthread_mutex_unlock(&c->mutex));
    if(data[1]!=0xa5)return RZ_INVALID;
    for(size_t i=6;i<n;++i)if(data[i]!=(unsigned char)i)return RZ_INVALID;
    *value=(unsigned)data[2]<<24|(unsigned)data[3]<<16|(unsigned)data[4]<<8|data[5];return RZ_OK;
}
static void blob_dispose(void *p,unsigned char *data) {
    struct rz_context *c=p;unsigned tag=data[0],bit=1u<<tag;
    CHECK(!pthread_mutex_lock(&c->mutex));c->dispose_enter|=bit;CHECK(!pthread_cond_broadcast(&c->changed));
    while((c->dispose_pause&bit) && !(c->dispose_release&bit))CHECK(!pthread_cond_wait(&c->changed,&c->mutex));
    CHECK(!pthread_mutex_unlock(&c->mutex));atomic_fetch_add(&c->freed[tag],1);free(data);
}
static struct rzh_blob blob(struct rz_context *c,unsigned tag,bool valid,unsigned value,size_t n) {
    CHECK(n>=6);unsigned char *p=malloc(n);CHECK(p);p[0]=(unsigned char)tag;p[1]=valid?0xa5:0;
    p[2]=(unsigned char)(value>>24);p[3]=(unsigned char)(value>>16);p[4]=(unsigned char)(value>>8);p[5]=(unsigned char)value;
    for(size_t i=6;i<n;++i)p[i]=(unsigned char)i;
    return (struct rzh_blob){p,n,c,blob_dispose};
}
static void drop(struct rzh_blob *b){if(b->data)b->dispose(b->context,b->data);*b=(struct rzh_blob){0};}
static void *allocate(void *p,size_t n){(void)p;return malloc(n);}
static void deallocate(void *p,void *v){atomic_fetch_add(&((struct rz_context *)p)->scopes,1);free(v);}
static struct rzf_scope *scope(struct rz_context *c,uint64_t deadline) {
    struct rzf_scope *s=NULL;CHECK(rzf_scope_create(NULL,0,deadline,(struct rzf_allocator){c,allocate,deallocate},&s)==RZ_OK);return s;
}
static void *worker(void *p) {
    struct rz_worker *w=p;w->status=rzh_complete(w->arbiter,w->ticket,&w->blob);
    release_mask(w->context,&w->context->done,w->bit);return NULL;
}
static void setup(struct rz_context *c,struct rzh_arbiter *a,struct rzf_scope **s,struct rzh_ticket t[2],uint64_t deadline) {
    context_init(c);CHECK(rzh_init(a,c,clock_read,decode)==RZ_OK);*s=scope(c,deadline);
    CHECK(rzh_begin(a,*s,2,0,1,1,&t[0])==RZ_OK);
    struct rzh_ticket before={0};CHECK(rzh_start(a,&before)==RZ_PENDING && !before.owner);
    atomic_store(&c->time,2);CHECK(rzh_start(a,&t[1])==RZ_OK);
}
static void ordered(void) {
    for(unsigned first=0;first<2;++first)for(unsigned accepted=0;accepted<4;++accepted) {
        struct rz_context c;struct rzh_arbiter a;struct rzf_scope *s;struct rzh_ticket t[2];setup(&c,&a,&s,t,100);c.decode_pause=3;
        int value=0;bool duplicate=true;
        CHECK(rzh_effect(&a,t[0],3,&value,&duplicate)==RZ_OK && value==3 && !duplicate);
        CHECK(rzh_effect(&a,t[1],3,&value,&duplicate)==RZ_OK && value==3 && duplicate);
        value=99;CHECK(rzh_effect(&a,t[1],4,&value,&duplicate)==RZ_CONFLICT && value==99);
        struct rz_worker workers[2];pthread_t threads[2];
        for(unsigned i=0;i<2;++i){workers[i]=(struct rz_worker){&a,t[i],blob(&c,i,(accepted&(1u<<i))!=0,100+i,6+7*i),&c,1u<<i,0};CHECK(!pthread_create(&threads[i],NULL,worker,&workers[i]));}
        wait_mask(&c,&c.decode_enter,3);release_mask(&c,&c.decode_release,1u<<first);wait_mask(&c,&c.done,1u<<first);
        CHECK(rzh_quiescent(&a)==RZ_BUSY);release_mask(&c,&c.decode_release,1u<<!first);
        for(unsigned i=0;i<2;++i)CHECK(!pthread_join(threads[i],NULL));
        CHECK(rzh_quiescent(&a)==RZ_OK);struct rzh_snapshot snapshot;CHECK(rzh_snapshot(&a,&snapshot)==RZ_OK && snapshot.total==3 && snapshot.permits==0);
        int winner=(accepted&(1u<<first))?(int)first:((accepted&(1u<<!first))?(int)!first:-1);
        CHECK(snapshot.winner==winner);struct rzh_result result={0};
        CHECK(rzh_take(&a,&result)==(winner<0?RZ_PENDING:RZ_OK));
        if(winner>=0){CHECK(result.attempt==(unsigned)winner && result.value==100+(unsigned)winner);drop(&result.blob);CHECK(rzh_take(&a,&result)==RZ_DONE);}
        CHECK(atomic_load(&c.freed[0])==1 && atomic_load(&c.freed[1])==1);
        struct rzh_blob duplicate_blob=blob(&c,0,true,9,6);CHECK(rzh_complete(&a,t[0],&duplicate_blob)==RZ_STALE && duplicate_blob.data);drop(&duplicate_blob);
        rzf_scope_release(&s);CHECK(!atomic_load(&c.scopes));CHECK(rzh_destroy(&a)==RZ_OK && atomic_load(&c.scopes)==1);context_destroy(&c);
    }
}
static void simultaneous(void) {
    for(unsigned round=0;round<32;++round) {
        struct rz_context c;struct rzh_arbiter a;struct rzf_scope *s;struct rzh_ticket t[2];setup(&c,&a,&s,t,100);c.decode_pause=3;
        struct rz_worker workers[2];pthread_t threads[2];
        for(unsigned i=0;i<2;++i){workers[i]=(struct rz_worker){&a,t[i],blob(&c,i,true,30+i,17+i),&c,1u<<i,0};CHECK(!pthread_create(&threads[i],NULL,worker,&workers[i]));}
        wait_mask(&c,&c.decode_enter,3);release_mask(&c,&c.decode_release,3);
        for(unsigned i=0;i<2;++i)CHECK(!pthread_join(threads[i],NULL));
        CHECK((workers[0].status==RZ_OK)+(workers[1].status==RZ_OK)==1);
        struct rzh_result result={0};CHECK(rzh_take(&a,&result)==RZ_OK && result.value==30+result.attempt);drop(&result.blob);
        CHECK(rzh_quiescent(&a)==RZ_OK && atomic_load(&c.freed[0])==1 && atomic_load(&c.freed[1])==1);
        rzf_scope_release(&s);CHECK(rzh_destroy(&a)==RZ_OK);context_destroy(&c);
    }
}
static void cleanup_and_reuse(void) {
    struct rz_context c;struct rzh_arbiter a;struct rzf_scope *s;struct rzh_ticket t[2];setup(&c,&a,&s,t,100);
    struct rzh_blob winning=blob(&c,0,true,44,40);CHECK(rzh_complete(&a,t[0],&winning)==RZ_OK && !winning.data);
    struct rzh_result result={0};CHECK(rzh_take(&a,&result)==RZ_OK && result.value==44);drop(&result.blob);
    c.dispose_pause=2;struct rz_worker late={&a,t[1],blob(&c,1,true,7,120),&c,2,0};late.blob.data[119]^=1;
    pthread_t thread;CHECK(!pthread_create(&thread,NULL,worker,&late));
    wait_mask(&c,&c.dispose_enter,2);struct rzh_snapshot snapshot;CHECK(rzh_snapshot(&a,&snapshot)==RZ_OK && snapshot.completed==3 && snapshot.callbacks==1 && snapshot.permits==1);
    CHECK(rzh_quiescent(&a)==RZ_BUSY && rzh_destroy(&a)==RZ_BUSY);
    struct rzh_ticket next={0};CHECK(rzh_begin(&a,s,1,0,2,1,&next)==RZ_BUSY && !next.owner);
    CHECK(rzh_abandon(&a)==RZ_OK);release_mask(&c,&c.dispose_release,2);CHECK(!pthread_join(thread,NULL));
    CHECK(a.gate.health[0].count==2 && a.gate.health[0].failures==1); /* Published failed decode is not revised by abandonment during disposal. */
    CHECK(rzh_quiescent(&a)==RZ_OK);CHECK(rzh_begin(&a,s,1,0,2,1,&next)==RZ_OK && next.generation==t[0].generation+1);
    struct rzh_blob stale=blob(&c,0,true,999,8);CHECK(rzh_complete(&a,t[0],&stale)==RZ_STALE && stale.data);drop(&stale);
    struct rzh_blob fresh=blob(&c,0,true,55,15);CHECK(rzh_complete(&a,next,&fresh)==RZ_OK);
    CHECK(rzh_take(&a,&result)==RZ_OK && result.value==55);drop(&result.blob);
    a.generation=UINT64_MAX;next=(struct rzh_ticket){0};CHECK(rzh_begin(&a,s,1,0,3,1,&next)==RZ_FULL && !next.owner);
    rzf_scope_release(&s);CHECK(rzh_destroy(&a)==RZ_OK);context_destroy(&c);
}
static void cancellation_and_decode(void) {
    for(unsigned mode=0;mode<3;++mode) {
        struct rz_context c;struct rzh_arbiter a;struct rzf_scope *s;struct rzh_ticket t[2];setup(&c,&a,&s,t,5);c.decode_pause=3;
        struct rz_worker w[2];pthread_t threads[2];
        for(unsigned i=0;i<2;++i){w[i]=(struct rz_worker){&a,t[i],blob(&c,i,true,i+1,10+i),&c,1u<<i,0};CHECK(!pthread_create(&threads[i],NULL,worker,&w[i]));}
        wait_mask(&c,&c.decode_enter,3);
        struct rzh_blob duplicate=blob(&c,0,true,7,6);CHECK(rzh_complete(&a,t[0],&duplicate)==RZ_BUSY && duplicate.data);drop(&duplicate);
        if(mode==0)CHECK(rzh_abandon(&a)==RZ_OK);
        else if(mode==1)atomic_store(&c.time,5);
        else rz_cancel(&s->core);
        bool cancelled=false;CHECK(rzh_cancelled(&a,t[1],&cancelled)==RZ_OK && cancelled);
        rzf_scope_release(&s);CHECK(!atomic_load(&c.scopes) && rzh_quiescent(&a)==RZ_BUSY);
        release_mask(&c,&c.decode_release,3);for(unsigned i=0;i<2;++i)CHECK(!pthread_join(threads[i],NULL));
        struct rzh_snapshot snapshot;CHECK(rzh_snapshot(&a,&snapshot)==RZ_OK && snapshot.winner==-1 && snapshot.finalized==3 && !snapshot.permits);
        CHECK(rzh_quiescent(&a)==RZ_OK && atomic_load(&c.freed[0])==2 && atomic_load(&c.freed[1])==1);
        CHECK(rzh_destroy(&a)==RZ_OK && atomic_load(&c.scopes)==1);context_destroy(&c);
    }
}
static void admission(void) {
    struct rz_context c;context_init(&c);struct rzh_arbiter a;CHECK(rzh_init(&a,&c,clock_read,decode)==RZ_OK);
    struct rzf_scope *s=scope(&c,100);struct rzh_ticket ticket={0};
    atomic_store(&s->references,SIZE_MAX);
    CHECK(rzh_begin(&a,s,1,0,1,1,&ticket)==RZ_FULL && !ticket.owner && !a.active && !a.gate.used[0]);
    atomic_store(&s->references,1);
    CHECK(rzh_begin(&a,s,1,0,0,1,&ticket)==RZ_INVALID && !ticket.owner);
    CHECK(rzh_begin(&a,s,1,0,1,1,&ticket)==RZ_OK);
    struct rzh_ticket other=ticket;other.owner=NULL;struct rzh_blob empty={0};
    CHECK(rzh_complete(&a,other,&empty)==RZ_STALE);
    other=ticket;other.attempt=1;CHECK(rzh_complete(&a,other,&empty)==RZ_STALE);
    CHECK(rzh_abandon(&a)==RZ_OK && rzh_abandon(&a)==RZ_OK);
    CHECK(rzh_complete(&a,ticket,&empty)==RZ_DONE && rzh_quiescent(&a)==RZ_OK);
    rzf_scope_release(&s);CHECK(rzh_destroy(&a)==RZ_OK && atomic_load(&c.scopes)==1);context_destroy(&c);
}
static void oracle(void) {
    char word[64];while(scanf("%63s",word)==1) {
        struct rz_context c;struct rzh_arbiter a;struct rzf_scope *s;struct rzh_ticket t[2];setup(&c,&a,&s,t,5);unsigned returned=999;
        for(size_t i=0;word[i];++i) {
            int status=RZ_OK;
            if(word[i]=='t')atomic_store(&c.time,5);
            else if(word[i]=='x')status=rzh_abandon(&a);
            else if(word[i]=='r') {
                struct rzh_result result={0};status=rzh_take(&a,&result);
                if(status==RZ_OK){returned=result.value;drop(&result.blob);}
            } else if(word[i]=='e' || word[i]=='f') {
                int value=0;bool duplicate;status=rzh_effect(&a,t[word[i]=='f'],1,&value,&duplicate);
            } else {
                unsigned attempt=word[i]=='b' || word[i]=='B';
                struct rzh_blob response=blob(&c,attempt,word[i]=='a' || word[i]=='b',10+attempt,6+attempt);
                status=rzh_complete(&a,t[attempt],&response);drop(&response);
            }
            struct rzh_snapshot snapshot;CHECK(rzh_snapshot(&a,&snapshot)==RZ_OK);
            printf("%d:%d:%u:%u:%u:%u:%u:%u:%u:%u:%d;",status,snapshot.winner,snapshot.completed,snapshot.permits,
                snapshot.cancel_requested,snapshot.abandoned,snapshot.taken,returned,
                atomic_load(&c.freed[0]),atomic_load(&c.freed[1]),snapshot.total);
        }
        putchar('\n');CHECK(rzh_abandon(&a)==RZ_OK);
        for(unsigned i=0;i<2;++i){struct rzh_blob empty={0};int status=rzh_complete(&a,t[i],&empty);CHECK(status==RZ_STALE || status==RZ_DONE || status==RZ_EXPIRED);}
        rzf_scope_release(&s);CHECK(rzh_destroy(&a)==RZ_OK);context_destroy(&c);
    }
}
static void population(void) {
    unsigned first,second,hedged;
    while(scanf("%u %u %u",&first,&second,&hedged)==3) {
        CHECK(first>=1 && first<=64 && second>=1 && second<=64 && hedged<=1);
        struct rz_context c;context_init(&c);struct rzh_arbiter a;CHECK(rzh_init(&a,&c,clock_read,decode)==RZ_OK);struct rzf_scope *s=scope(&c,20);struct rzh_ticket tickets[2];
        CHECK(rzh_begin(&a,s,4,0,1,1,&tickets[0])==RZ_OK);unsigned count=1,due[2]={first,4+second};
        if(hedged && first>4){atomic_store(&c.time,4);CHECK(rzh_start(&a,&tickets[1])==RZ_OK);count=2;}
        unsigned order[2]={0,1};if(count==2 && due[1]<due[0]){order[0]=1;order[1]=0;}
        int latency=-1;struct rzh_snapshot snapshot;
        for(unsigned i=0;i<count;++i) {
            unsigned attempt=order[i];atomic_store(&c.time,due[attempt]);struct rzh_blob response=blob(&c,attempt,true,attempt,6+attempt);
            int status=rzh_complete(&a,tickets[attempt],&response);CHECK(status==RZ_OK || status==RZ_DONE || status==RZ_EXPIRED);
            CHECK(rzh_snapshot(&a,&snapshot)==RZ_OK);if(latency<0 && snapshot.winner>=0)latency=(int)due[attempt];
        }
        struct rzh_result result={0};int status=rzh_take(&a,&result);CHECK(status==(latency<0?RZ_PENDING:RZ_OK));drop(&result.blob);
        CHECK(rzh_quiescent(&a)==RZ_OK);printf("%d %u %u %u %d\n",latency,count,first+(count==2?second:0),due[order[count-1]],snapshot.winner);
        rzf_scope_release(&s);CHECK(rzh_destroy(&a)==RZ_OK);context_destroy(&c);
    }
}
int main(int argc,char **argv) {
    if(argc==2 && !strcmp(argv[1],"oracle")){oracle();return 0;}
    if(argc==2 && !strcmp(argv[1],"population")){population();return 0;}
    ordered();simultaneous();cleanup_and_reuse();cancellation_and_decode();admission();puts("owned hedge tests passed");return 0;
}
