#include "protocols.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define C(x) do { if (!(x)) { fprintf(stderr,"PB_CHECK:%d:%s\n",__LINE__,#x); return 1; } } while (0)
static int lifecycle(void) {
    for (unsigned fail=0; fail<3; ++fail) for (unsigned quiet=0; quiet<2; ++quiet) {
        struct pt_device d; struct pt_closed c,old; struct pt_ready out={NULL,91};
        pt_device_init(&d,&c); old=c;
        C(pt_configure(&c,fail,&out)==(fail?PT_RECOVERY:PT_OK));
        C(!c.device && d.generation==2 && d.control==(fail==1?1u:3u));
        if (fail) {
            C(!out.device && out.generation==91 && d.state==PT_RECOVERY_REQUIRED);
            struct pt_closed recovered={NULL,77}; struct pt_device before=d;
            C(pt_recover(&d,quiet,&recovered)==(quiet?PT_OK:PT_PENDING));
            if (!quiet) C(!memcmp(&d,&before,sizeof d)&&!recovered.device&&recovered.generation==77);
            else { C(d.cleanup==1 && d.control==0 && recovered.generation==3); C(pt_configure(&old,0,&out)==PT_STATE); }
        } else { struct pt_ready copied=out; C(pt_close(&out,&c)==PT_OK); C(pt_close(&copied,&c)==PT_STATE); }
    }
    struct pt_device d; struct pt_closed c; struct pt_ready out={NULL,9}; pt_device_init(&d,&c);
    d.generation=c.generation=UINT32_MAX;
    C(pt_configure(&c,0,&out)==PT_RANGE && c.device==&d && out.generation==9);
    d.state=PT_RECOVERY_REQUIRED; d.control=3;
    C(pt_recover(&d,1,&c)==PT_RANGE && d.control==3 && !d.cleanup);
    d.state=PT_READY; out=(struct pt_ready){&d,UINT32_MAX};
    C(pt_close(&out,&c)==PT_RANGE && out.device==&d && d.control==3);
    return 0;
}
static int builders(void) {
    static const unsigned order[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    const unsigned value[3]={252,4,1000};
    for(unsigned p=0;p<6;p++) {
        struct pt_builder b={0}; struct pt_config out={91,92,93};
        for(unsigned i=0;i<3;i++) {
            C(pt_builder_finish(&b,&out)==PT_STATE && out.address==91);
            unsigned f=order[p][i]; C(pt_builder_set(&b,f,value[f])==PT_OK);
            struct pt_builder before=b; C(pt_builder_set(&b,f,value[f])==PT_STATE&&!memcmp(&b,&before,sizeof b));
        }
        C(pt_builder_finish(&b,&out)==PT_OK&&out.address==252&&out.length==4&&out.timeout==1000);
    }
    return 0;
}
static int stateful(void) {
    struct pt_machine m={PT_IDLE,0,UINT32_MAX};unsigned char trace[6]={99};size_t used=71;
    C(pt_dispatch(&m,PT_PING,trace,6,&used)==PT_RANGE&&used==71&&trace[0]==99&&m.pings==UINT32_MAX);
    struct pt_active q={{PT_OFFLINE,0,0},{0},0,0};
    for(unsigned i=0;i<101;i++) {
        const unsigned ev[]={PT_CONNECT,PT_START,PT_FINISH,PT_FAIL};
        for(unsigned j=0;j<4;j++) C(pt_post(&q,ev[j])==PT_OK);
        struct pt_active before=q; C(pt_post(&q,PT_RESET)==PT_RANGE&&!memcmp(&q,&before,sizeof q));
        C(pt_active_step(&q,trace,0,&used)==PT_RANGE&&!memcmp(&q,&before,sizeof q));
        for(unsigned j=0;j<4;j++) C(pt_active_step(&q,trace,6,&used)==PT_OK);
        C(pt_post(&q,PT_RESET)==PT_OK&&pt_active_step(&q,trace,6,&used)==PT_OK&&q.machine.active==PT_OFFLINE);
    }
    struct pt_thread a={0},b={0};uint32_t oa=91,ob=92;
    for(unsigned i=0;i<3;i++) {
        C(pt_thread_sum(&a,0,99,&oa)==PT_PENDING&&a.count==i);
        C(pt_thread_sum(&b,0,99,&ob)==PT_PENDING&&b.count==i);
        C(pt_thread_sum(&a,1,i+1,&oa)==(i==2?PT_DONE:PT_PENDING));
        C(pt_thread_sum(&b,1,10*(i+1),&ob)==(i==2?PT_DONE:PT_PENDING));
    }
    C(oa==6&&ob==60);oa=77;ob=88;
    C(pt_thread_sum(&a,0,0,&oa)==PT_DONE&&pt_thread_sum(&b,1,99,&ob)==PT_DONE&&oa==77&&ob==88);
    return 0;
}
struct pf_state { struct pt_behavior *b;const struct pt_node *nodes;unsigned calls[5],aborts[5],bad; };
static unsigned nested(void *v,unsigned n) {
    struct pf_state *s=v;s->calls[n]++;unsigned sentinel=91;
    if(pt_behavior_tick(s->b,s->nodes,5,nested,s,&sentinel)!=PT_STATE||sentinel!=91)s->bad=1;
    return n==2?PT_BT_RUNNING:n==3?PT_BT_FAILURE:PT_BT_SUCCESS;
}
static void stop(void *v,unsigned n) {struct pf_state*s=v;s->aborts[n]++;}
static int trees(void) {
    /* Root sequence: nested selector (failure, success), then running leaf. */
    struct pt_node n[]={{PT_BT_SEQUENCE,1,2},{PT_BT_SELECTOR,3,2},{PT_BT_LEAF,0,0},{PT_BT_LEAF,0,0},{PT_BT_LEAF,0,0}};
    struct pt_behavior b={0};struct pf_state s={&b,n,{0},{0},0};unsigned out=91;
    C(pt_behavior_tick(&b,n,5,nested,&s,&out)==PT_OK&&out==PT_BT_RUNNING&&!s.bad);
    C(s.calls[3]==1&&s.calls[4]==1&&s.calls[2]==1);
    C(pt_behavior_tick(&b,n,5,nested,&s,&out)==PT_OK&&s.calls[3]==1&&s.calls[4]==1&&s.calls[2]==2);
    C(pt_behavior_cancel(&b,stop,&s)==PT_OK&&s.aborts[2]==1&&!s.aborts[3]&&!s.aborts[4]);
    C(pt_behavior_cancel(&b,stop,&s)==PT_OK&&s.aborts[2]==1);
    n[1].first=2;C(pt_behavior_tick(&b,n,5,nested,&s,&out)==PT_INVALID);
    return 0;
}
int main(void) { return lifecycle()||builders()||stateful()||trees(); }
