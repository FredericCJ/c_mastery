#include "breaker.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do{if(!(c)){fprintf(stderr,"RZ_CHECK: %s:%d %s\n",__FILE__,__LINE__,#c);exit(1);}}while(0)
static void *allocate(void *p,size_t n){(void)p;return malloc(n);}
static void deallocate(void *p,void *v){++*(unsigned *)p;free(v);}
static RzfScope *make_scope(unsigned *freed) {
    RzfScope *s=NULL;CHECK(rzf_scope_create(NULL,0,UINT64_MAX,(RzfAllocator){freed,allocate,deallocate},&s)==RZ_OK);return s;
}
static void invariant(const RzbGate *g) {
    for(unsigned t=0;t<2;++t) {
        unsigned used=0,failures=0;
        for(unsigned i=t*2;i<t*2+2;++i)if(g->slots[i].active)used+=g->slots[i].cost;
        CHECK(used==g->used[t] && used<=g->reserved[t]);
        for(unsigned i=0;i<g->policy.window;++i)failures+=g->health[t].samples[i];
        CHECK(failures==g->health[t].failures && g->health[t].count<=g->policy.window);
    }
}
static void cycle(RzbGate *g,RzfScope *s,unsigned tenant,uint64_t now,int outcome) {
    RzbPermit p;CHECK(rzb_admit(g,tenant,1,s,now,&p)==RZ_OK);
    CHECK(rzb_complete(g,p,now,outcome)==RZ_OK);invariant(g);
    CHECK(rzb_complete(g,p,now,outcome)==RZ_STALE);
}
static void tests(void) {
    unsigned freed=0;RzfScope *s=make_scope(&freed);RzbGate g;RzbPermit a,b,c;
    CHECK(rzb_init(&g,2,2,(RzbPolicy){4,4,3,4},0)==RZ_OK);
    const int history[]={0,1,1,1,1};
    for(unsigned i=0;i<5;++i)cycle(&g,s,0,i,history[i]);
    CHECK(g.health[0].count==4 && g.health[0].failures==0);

    CHECK(rzb_init(&g,2,2,(RzbPolicy){2,2,1,2},0)==RZ_OK);
    CHECK(rzb_admit(&g,0,1,s,0,&a)==RZ_OK);
    cycle(&g,s,0,0,0);cycle(&g,s,0,0,1);CHECK(g.health[0].phase==1 && g.health[0].until==4);
    CHECK(rzb_admit(&g,0,1,s,3,&c)==RZ_OPEN);
    CHECK(rzb_admit(&g,0,1,s,4,&b)==RZ_OK && g.health[0].phase==2);
    CHECK(rzb_complete(&g,a,4,1)==RZ_OK && g.health[0].phase==2 && g.used[0]==1);
    CHECK(rzb_admit(&g,0,1,s,4,&c)==RZ_BUSY);
    CHECK(rzb_complete(&g,b,4,1)==RZ_OK && g.health[0].phase==0 && !g.health[0].count);
    CHECK(rzb_admit(&g,0,2,s,4,&a)==RZ_OK && rzb_admit(&g,0,1,s,4,&b)==RZ_BUSY);
    CHECK(rzb_admit(&g,1,2,s,4,&b)==RZ_OK);invariant(&g);
    CHECK(rzb_cancel(&g,a)==RZ_OK && g.used[0]==2);
    CHECK(rzb_admit(&g,0,1,s,4,&c)==RZ_BUSY);
    CHECK(rzb_close(&g)==RZ_OK && rzb_quiescent(&g)==RZ_BUSY);
    bool cancelled=false;CHECK(rzb_cancelled(&g,b,4,&cancelled)==RZ_OK && cancelled);
    CHECK(rzb_complete(&g,a,4,-1)==RZ_OK && g.used[1]==2 && rzb_quiescent(&g)==RZ_BUSY);
    rzf_scope_release(&s);CHECK(!freed);
    CHECK(rzb_complete(&g,b,8,-1)==RZ_OK && rzb_quiescent(&g)==RZ_OK && freed==1);
    CHECK(rzb_cancel(&g,a)==RZ_STALE);

    s=make_scope(&freed);
    for(unsigned order=0;order<2;++order)for(int first=-1;first<=1;++first)for(int second=-1;second<=1;++second) {
        CHECK(rzb_init(&g,2,2,(RzbPolicy){4,4,1,2},0)==RZ_OK);
        RzbPermit p[2];CHECK(rzb_admit(&g,0,1,s,0,&p[0])==RZ_OK && rzb_admit(&g,0,1,s,0,&p[1])==RZ_OK);
        CHECK(rzb_complete(&g,p[order],0,first)==RZ_OK && rzb_complete(&g,p[!order],0,second)==RZ_OK);invariant(&g);
        CHECK(g.health[0].count==(unsigned)((first>=0)+(second>=0)));
    }
    CHECK(rzb_init(&g,2,2,(RzbPolicy){1,1,1,1},0)==RZ_OK);
    g.health[0].epoch=UINT_MAX;cycle(&g,s,0,0,0);
    CHECK(g.health[0].retired && rzb_admit(&g,0,1,s,4,&a)==RZ_OPEN);
    cycle(&g,s,1,4,1);CHECK(!g.health[1].retired);
    CHECK(rzb_init(&g,2,2,(RzbPolicy){1,1,1,1},0)==RZ_OK);
    cycle(&g,s,0,UINT64_MAX-2,0);CHECK(g.health[0].retired);
    CHECK(rzb_init(&g,2,2,(RzbPolicy){4,4,1,2},0)==RZ_OK);
    g.slots[0].generation=g.slots[1].generation=UINT_MAX-1;
    cycle(&g,s,0,0,1);cycle(&g,s,0,0,1);a=(RzbPermit){0};
    CHECK(rzb_admit(&g,0,1,s,0,&a)==RZ_FULL && !a.owner);
    CHECK(rzb_admit(&g,1,1,s,0,&a)==RZ_OK);
    unsigned char before[sizeof g];memcpy(before,&g,sizeof g);
    CHECK(rzb_complete(&g,a,0,2)==RZ_INVALID && !memcmp(&g,before,sizeof g));
    CHECK(rzb_complete(&g,a,0,-1)==RZ_OK);rzf_scope_release(&s);CHECK(freed==2);

    /* Same F,S,F,S observations: these intentionally different health contracts disagree. */
    s=make_scope(&freed);CHECK(rzb_init(&g,2,2,(RzbPolicy){4,4,1,2},0)==RZ_OK);RZGate original;
    CHECK(rz_gate_init(&original,2,0)==RZ_OK);
    for(unsigned i=0;i<4;++i){RZPermit p;CHECK(rz_admit(&original,0,&p)==RZ_OK);CHECK(rz_finish(&original,p,0,i%2)==RZ_OK);cycle(&g,s,0,0,(int)(i%2));}
    CHECK(!original.phase && g.health[0].phase==1);rzf_scope_release(&s);
}
static void oracle(void) {
    unsigned window,minimum,numerator,denominator;char word[64];
    while(scanf("%u %u %u %u %63s",&window,&minimum,&numerator,&denominator,word)==5) {
        unsigned freed=0;RzfScope *s=make_scope(&freed);RzbGate g;CHECK(rzb_init(&g,2,2,(RzbPolicy){window,minimum,numerator,denominator},0)==RZ_OK);
        RzbPermit last[2]={{0},{0}};uint64_t now=0;
        for(size_t i=0;word[i];++i) {
            int status=RZ_OK;unsigned tenant=word[i]>='A' && word[i]<='Z';char op=tenant?(char)(word[i]-'A'+'a'):word[i];
            if(op=='a' || op=='b')status=rzb_admit(&g,tenant,op=='a'?1:2,s,now,&last[tenant]);
            else if(op=='f' || op=='s' || op=='n') {
                unsigned slot=tenant*2;while(slot<tenant*2+2 && !g.slots[slot].active)++slot;
                RzbPermit p=slot<tenant*2+2 ? (RzbPermit){&g,slot,g.slots[slot].generation} : last[tenant];
                status=rzb_complete(&g,p,now,op=='f'?0:op=='s'?1:-1);
            } else if(op=='c')status=rzb_cancel(&g,last[tenant]);
            else if(op=='d')status=rzb_complete(&g,last[tenant],now,1);
            else if(op=='t')now+=4;
            else if(op=='x')status=rzb_close(&g);
            else CHECK(false);
            invariant(&g);
            printf("%d:%u:%u",status,g.used[0],g.used[1]);
            for(unsigned t=0;t<2;++t)printf(":%u:%u:%u:%u:%"PRIu64,g.health[t].phase,g.health[t].epoch,g.health[t].count,g.health[t].failures,g.health[t].until);
            printf(";");
        }
        for(unsigned i=0;i<4;++i)if(g.slots[i].active)CHECK(rzb_complete(&g,(RzbPermit){&g,i,g.slots[i].generation},now,-1)==RZ_OK);
        rzf_scope_release(&s);CHECK(freed==1);puts("");
    }
}
int main(int argc,char **argv){CHECK(argc==2);if(!strcmp(argv[1],"test"))tests();else if(!strcmp(argv[1],"oracle"))oracle();else return 2;return 0;}
