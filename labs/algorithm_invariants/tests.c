#include "invariants.h"
#include "instrument.h"
#include <stdio.h>
#include <string.h>
AICounts ai_ring_counts,ai_link_counts;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"AI_ASSERT:%s\n",#x);return 41;}}while(0)
static unsigned divisor_gcd(unsigned a,unsigned b){unsigned best=0;for(unsigned d=1;d<=4095;++d)if(a%d==0 && b%d==0)best=d;return best;}
static int euclid(void){
    AIState state={1071,462},next={99,88};unsigned count=0;const AIState expected[3]={{462,147},{147,21},{21,0}};
    for(unsigned i=0;i<3;++i){CHECK(ai_step(state,0,&next)==AI_OK && next.a==expected[i].a && next.b==expected[i].b);state=next;}
    CHECK(ai_step(state,0,&next)==AI_DONE && next.a==21 && next.b==0);
    state=(AIState){1071,462};for(;;){enum AIStatus s=ai_step(state,1,&next);if(s==AI_DONE)break;CHECK(s==AI_OK && next.a+next.b<state.a+state.b);state=next;CHECK(++count<8190);}
    CHECK(state.a==21 && state.b==21 && count==11);
    unsigned cases=0;for(unsigned a=0;a<64;++a)for(unsigned b=0;b<64;++b)if(a||b)for(unsigned mode=0;mode<2;++mode){
        state=(AIState){a,b};count=0;for(;;){enum AIStatus status=ai_step(state,mode,&next);if(status==AI_DONE)break;CHECK(status==AI_OK);if(mode)CHECK(next.a+next.b<state.a+state.b);else CHECK(next.b<state.b);state=next;CHECK(++count<128);}
        CHECK((state.a?state.a:state.b)==divisor_gcd(a,b));++cases;
    }
    next=(AIState){99,88};CHECK(ai_step((AIState){0,0},0,&next)==AI_INVALID && next.a==99 && next.b==88);
    CHECK(ai_step((AIState){4096,1},0,&next)==AI_INVALID);CHECK(ai_step((AIState){1,1},2,&next)==AI_INVALID);
    state=(AIState){4095,1};count=0;while(ai_step(state,1,&next)==AI_OK){state=next;++count;}CHECK(count==4094 && state.a==1 && state.b==1);
    printf("OBS euclid pairs=%u modulo=3 subtraction=11 maximal-subtractions=4094 no-zero-divisor\n",cases);return 0;
}
static int growth(void){
    AIGrowth d,a;CHECK(ai_growth(8,0,&d)==AI_OK && d.copies==7 && d.writes==15 && d.worst==5 && d.capacity==8 && d.acquisitions==4 && d.releases==4);
    CHECK(ai_growth(8,1,&a)==AI_OK && a.copies==28 && a.writes==36 && a.worst==8 && a.acquisitions==8 && a.releases==8);
    for(unsigned n=0;n<=32;++n){CHECK(ai_growth(n,1,&a)==AI_OK && a.writes==(size_t)n*(n+1)/2 && a.worst==n && a.acquisitions==a.releases);CHECK(ai_growth(n,0,&d)==AI_OK && d.writes<(size_t)3*n+1 && d.acquisitions==d.releases);}
    a=(AIGrowth){99,98,97,96,95,94};CHECK(ai_growth(33,0,&a)==AI_INVALID && a.writes==98);
    puts("OBS growth actual-CDVector doubling15 worst5 additive36 worst8 all-counts0..32");return 0;
}
static int permutation(void){
    const AIRecord before[2]={{2,0},{1,0}},lost[2]={{1,0},{1,0}},sorted[2]={{1,0},{2,0}};
    const AIRecord tagged[3]={{1,10},{1,10},{1,20}},corrupt[3]={{1,10},{1,20},{1,20}};unsigned out=99;
    CHECK(ai_permutation(before,lost,2,&out)==AI_OK && !out);CHECK(ai_permutation(before,sorted,2,&out)==AI_OK && out);
    CHECK(ai_permutation(tagged,corrupt,3,&out)==AI_OK && !out);
    CHECK(ai_permutation(NULL,NULL,0,&out)==AI_OK && out);
    out=99;CHECK(ai_permutation(NULL,sorted,1,&out)==AI_INVALID && out==99);
    puts("OBS permutation sorted-prefix-loses-value equal-key-tag-multiplicity defeats-set");return 0;
}
static int queues(void){
    AIQueues q;memset(&q,0,sizeof q);ai_queues_init(&q);int removed=99;
    const unsigned put[7]={1,1,0,1,1,1,0};const int values[7]={11,22,0,33,44,55,0};const unsigned counts[7]={1,2,1,2,3,3,2};const int logical[7][3]={{11},{11,22},{22},{22,33},{22,33,44},{22,33,44},{33,44}};
    for(unsigned event=0;event<7;++event){
        ai_ring_counts=(AICounts){0};ai_link_counts=(AICounts){0};CDQueue old=q.queue;CDRing ring=q.ring;int old_backing[3];memcpy(old_backing,q.backing,sizeof old_backing);
        CHECK(ai_queue_event(&q,put[event],values[event],&removed)==(event==5?CD_FULL:CD_OK));
        CHECK(q.ring.count==counts[event] && q.queue.count==counts[event]);int at=q.queue.head;
        for(unsigned i=0;i<counts[event];++i){CHECK(q.backing[(q.ring.head+i)%3]==logical[event][i]);CHECK(at>=0 && at<3 && q.queue.nodes[at].live && q.queue.nodes[at].value==logical[event][i]);at=q.queue.nodes[at].next;}
        CHECK(at==-1);if(event==2)CHECK(removed==11);if(event==6)CHECK(removed==22);
        if(event==5)CHECK(!memcmp(&old,&q.queue,sizeof old) && !memcmp(&ring,&q.ring,sizeof ring) && !memcmp(old_backing,q.backing,sizeof old_backing));
        printf("QUEUE %u head=%zu count=%zu nodes=%d,%d ring=%u,%u,%u,%u linked=%u,%u,%u,%u\n",event,q.ring.head,q.ring.count,q.queue.head,q.queue.tail,ai_ring_counts.decisions,ai_ring_counts.transfers,ai_ring_counts.slot_probes,ai_ring_counts.links,ai_link_counts.decisions,ai_link_counts.transfers,ai_link_counts.slot_probes,ai_link_counts.links);
#ifdef AI_INSTRUMENTED
        const unsigned ring_decisions[7]={20,20,7,20,20,5,7},link_decisions[7]={4,5,3,4,6,3,3},slot_probes[7]={1,2,0,1,3,0,0};
        CHECK(ai_ring_counts.decisions==ring_decisions[event] && ai_link_counts.decisions==link_decisions[event]);
        CHECK(ai_link_counts.slot_probes==slot_probes[event] && ai_link_counts.links==(put[event]?0u:1u));
        CHECK(ai_ring_counts.decisions>0 && ai_link_counts.decisions>0);
        CHECK(ai_ring_counts.transfers==(event==5?0u:1u) && ai_link_counts.transfers==(event==5?0u:1u));
        if(event==5)CHECK(ai_link_counts.slot_probes==0 && ai_link_counts.links==0);
#endif
    }
    CHECK(ai_queue_event(&q,0,0,&removed)==CD_OK && removed==33);CHECK(ai_queue_event(&q,0,0,&removed)==CD_OK && removed==44);
    removed=99;CHECK(ai_queue_event(&q,0,0,&removed)==CD_EMPTY && removed==99);
    puts("OBS queues frozen-ring-and-node-providers exact-seven-state full-unchanged empty-unchanged");return 0;
}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"euclid"))return euclid();if(!strcmp(argv[1],"growth"))return growth();if(!strcmp(argv[1],"permutation"))return permutation();if(!strcmp(argv[1],"queues"))return queues();return 2;}
