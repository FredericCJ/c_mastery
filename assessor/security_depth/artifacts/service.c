#include "artifacts.h"
#include "worker.h"
#include "secrets.h"
#include <errno.h>
#include <string.h>
void sa_service_init(SAService *s) {
    sc_service_init(&s->queue);s->used_bytes=0;
    for(unsigned i=0;i<3;++i){s->charges[i]=s->resource[i]=0;memset(s->staging[i],0,4);}
}
enum sc_result sa_verified_submit(SAService *s,const SATrust *trust,const SAArtifact *artifact,const unsigned char *payload,size_t payload_size,const SCAuthority *authority,const unsigned char *request,size_t request_size,SCTicket *ticket) {
    SAReport report;
    if(sa_verify(trust,artifact,payload,payload_size,&report))return SC_INVALID;
    return sa_submit(s,&report,authority,request,request_size,ticket);
}
enum sc_result sa_submit(SAService *s,const SAReport *report,const SCAuthority *a,const unsigned char *bytes,size_t n,SCTicket *ticket) {
    SCBatch b={0};SCMeter meter;unsigned charge=0,resource=0;enum sc_result result;SCTicket next;
    if(!s || !report || !a || !ticket || (!bytes && n))return SC_INVALID;
    if(!report->ready)return SC_DENIED;
    if(n && bytes[0]==6) {
        unsigned width,count;
        if(n<3)return SC_LENGTH;
        width=bytes[1];count=bytes[2];if((width!=1 && width!=2) || count>4)return SC_COUNT;
        charge=width*count;if(n!=3+charge)return SC_LENGTH;
        if(!a->generation || a->allowed>1)return SC_INVALID;
        if(!a->allowed || a->generation==UINT_MAX)return SC_DENIED;
#ifndef SA_MUTANT_QUOTA
        if(charge>4-s->used_bytes)return SC_BUDGET;
#endif
        /* Independent physical bound remains even in the safe quota mutant. */
        if(charge>4)return SC_BUDGET;
        resource=1;b.count=1;b.work=count?count:1;b.commands[0].opcode=2;b.commands[0].count=count;b.commands[0].generation=a->generation;
    } else {
        result=sc_decode(bytes,n,a,1,&b,&meter);if(result!=SC_OK)return result;
    }
    result=sc_submit(&s->queue,&b,&next);if(result!=SC_OK)return result;
    s->charges[next.index]=charge;s->resource[next.index]=resource;s->used_bytes+=charge;
    if(charge)memcpy(s->staging[next.index],bytes+3,charge);
    *ticket=next;return SC_OK;
}
static void clear_slot(SAService *s,unsigned slot) {
    s->used_bytes-=s->charges[slot];s->charges[slot]=s->resource[slot]=0;ss_wipe(s->staging[slot],4);
}
enum sc_result sa_finish(SAService *s,SCTicket ticket,int cancel,const SCAuthority *a,int fd,uint32_t *ordinary,uint32_t *privileged,char out[4]) {
    char staged[4];enum sc_result result;int error=0;unsigned resource;
    if(!s || !a || !ordinary || !privileged || !out || (cancel!=0 && cancel!=1) || !a->generation || a->allowed>1)return SC_INVALID;
    if(ticket.index>=3 || !s->queue.jobs[ticket.index].active || s->queue.jobs[ticket.index].generation!=ticket.generation)return SC_STALE;
    resource=s->resource[ticket.index];
    if(resource && !cancel) {
        if(!a->allowed || a->generation!=s->queue.jobs[ticket.index].request.commands[0].generation)error=EACCES;
        else error=sd_request(1,SD_READ_LOG,fd,staged);
    }
    result=sc_finish(&s->queue,ticket,cancel || error,a,ordinary,privileged);
    if(result==SC_EXHAUSTED && s->queue.jobs[ticket.index].active){ss_wipe(staged,4);return result;}
    clear_slot(s,ticket.index);
    if(error){ss_wipe(staged,4);return error==EACCES?SC_DENIED:SC_INVALID;}
    if(result==SC_OK && resource && !cancel)memcpy(out,staged,4);
    ss_wipe(staged,4);return result;
}
enum sc_result sa_shutdown(SAService *s) {
    if(!s)return SC_INVALID;
    enum sc_result result=sc_shutdown(&s->queue);
    for(unsigned i=0;i<3;++i)clear_slot(s,i);
    return result;
}
