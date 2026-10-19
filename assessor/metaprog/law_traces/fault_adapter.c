#include "fault_adapter.h"
#include <stdlib.h>
#ifndef MPT_MUTATION
#define MPT_MUTATION 0
#endif
typedef struct {unsigned allocations,refusals,releases,live,fail_at,unexpected;void *blocks[3];} Schedule;
static void *allocate(void *context,size_t n){Schedule *s=context;unsigned i;void *p;++s->allocations;if(s->allocations==s->fail_at){++s->refusals;return NULL;}p=malloc(n);if(!p){++s->refusals;s->unexpected=1;return NULL;}for(i=0;i<3;++i)if(!s->blocks[i]){s->blocks[i]=p;++s->live;return p;}free(p);s->unexpected=1;return NULL;}
static void release(void *context,void *p){Schedule *s=context;unsigned i;for(i=0;i<3;++i)if(s->blocks[i]==p){s->blocks[i]=NULL;--s->live;++s->releases;free(p);return;}abort();}
static int shape(const MptTrace *t){size_t i;unsigned live[2]={0,0},allocations=0;if(!t||t->count>12||t->fail_at>12)return 0;for(i=0;i<t->count;++i){MptStep s=t->steps[i];if(s.operation>MPT_CHECK||s.slot>1||s.value>4||(s.operation==MPT_DROP&&s.value)||(s.operation==MPT_CHECK&&(s.slot||s.value)))return 0;if(s.operation==MPT_SET){if(++allocations!=t->fail_at)live[s.slot]=1;}else if(s.operation==MPT_DROP){if(!live[s.slot])return 0;live[s.slot]=0;}else if(!live[0]||!live[1])return 0;}return 1;}
static MptView value(unsigned v,unsigned cache){static const unsigned char a[]="A",b[]="a",c[]=" A ",d[]="B";static const unsigned char *const bytes[]={NULL,a,b,c,d};static const size_t lengths[]={0,1,1,3,1};return (MptView){bytes[v],lengths[v],cache};}
int mpt_replay(const MptTrace *t,MptLaws laws,MptOutcome *out){
    MptOwner owners[2]={{0},{0}};Schedule schedule={0};size_t i;int status=MPT_OK;MptOutcome result={MPT_NONE,0,0,0,0,0};
    if(!out||!laws.equal||!laws.hash||!shape(t))return MPT_INVALID;
    schedule.fail_at=t->fail_at;result.first_step=t->count;
    for(i=0;i<t->count;++i){MptStep step=t->steps[i];MptOwner *o=&owners[step.slot];
        if(step.operation==MPT_SET){MptOwner before=*o;unsigned char copy[8];size_t j;int assigned;for(j=0;j<before.length;++j)copy[j]=before.bytes[j];assigned=mpt_assign(o,value(step.value,step.slot+11),(MptAllocator){allocate,release,&schedule});
            if(assigned==MPT_NOMEM){unsigned changed=o->bytes!=before.bytes||o->length!=before.length||o->cache!=before.cache;if(!changed)for(j=0;j<before.length;++j)if(o->bytes[j]!=copy[j])changed=1;if(changed){result.failure=MPT_ROLLBACK;result.first_step=i;break;}}
            else if(assigned!=MPT_OK){result.failure=MPT_CALLBACK;result.first_step=i;break;}
        }else if(step.operation==MPT_DROP){if(!o->bytes){status=MPT_INVALID;break;}mpt_drop(o);}
        else{int equal;if(!owners[0].bytes||!owners[1].bytes){status=MPT_INVALID;break;}equal=laws.equal(laws.context,mpt_view(&owners[0]),mpt_view(&owners[1]));if(equal<0||equal>1){result.failure=MPT_CALLBACK;result.first_step=i;break;}if(equal&&laws.hash(laws.context,mpt_view(&owners[0]))!=laws.hash(laws.context,mpt_view(&owners[1]))){result.failure=MPT_HASH_LAW;result.first_step=i;break;}}
    }
    mpt_drop(&owners[0]);mpt_drop(&owners[1]);result.allocations=schedule.allocations;result.refusals=schedule.refusals;result.releases=schedule.releases;result.live=schedule.live;
    if(schedule.live){if(!result.failure){result.failure=MPT_CLEANUP;result.first_step=i;}for(i=0;i<3;++i)if(schedule.blocks[i])free(schedule.blocks[i]);}
    if(schedule.unexpected)status=MPT_NOMEM;
    if(status==MPT_OK)*out=result;
    return status;
}
static int preserves(const MptTrace *t,MptLaws laws,unsigned failure,unsigned *attempts){MptOutcome o={0};int status;++*attempts;status=mpt_replay(t,laws,&o);if(status==MPT_NOMEM)return -1;if(MPT_MUTATION==5)return status==MPT_INVALID||o.failure!=MPT_NONE;return status==MPT_OK&&o.failure==failure;}
int mpt_shrink(const MptTrace *input,MptLaws laws,MptTrace *out,unsigned *attempts){
    MptTrace current,trial;MptOutcome first;size_t i,j;unsigned tried=0;int changed,status;
    if(!out||!attempts)return MPT_INVALID;
    status=mpt_replay(input,laws,&first);if(status!=MPT_OK)return status;if(!first.failure)return MPT_NO_FAILURE;current=*input;
    if(MPT_MUTATION!=6)do{changed=0;
        for(i=0;i<current.count;++i){int kept;trial=current;for(j=i+1;j<current.count;++j)trial.steps[j-1]=trial.steps[j];--trial.count;kept=preserves(&trial,laws,first.failure,&tried);if(kept<0)return MPT_NOMEM;if(kept){current=trial;changed=1;break;}}
        if(changed)continue;
        for(i=0;i<current.count;++i)if(current.steps[i].operation==MPT_SET&&current.steps[i].value){int kept;trial=current;--trial.steps[i].value;kept=preserves(&trial,laws,first.failure,&tried);if(kept<0)return MPT_NOMEM;if(kept){current=trial;changed=1;break;}}
        if(changed)continue;
        if(current.fail_at>1){int kept;trial=current;--trial.fail_at;kept=preserves(&trial,laws,first.failure,&tried);if(kept<0)return MPT_NOMEM;if(kept){current=trial;changed=1;}}
    }while(changed);
    *out=current;*attempts=tried;return MPT_OK;
}
