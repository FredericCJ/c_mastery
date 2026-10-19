#include "ledger.h"
int af_admit(AfLedger *s,uint32_t epoch,uint32_t id){
 if(!s||!epoch||!id||id>128)return AF_INVALID;
 if(epoch!=s->epoch)return AF_STALE;
 if(id!=s->last+1)return AF_CONFLICT;
 s->item[id-1]=(AfRecord){epoch,AF_ACCEPTED};s->last=id;return AF_OK;
}
int af_terminal(AfLedger *s,uint32_t epoch,uint32_t id,unsigned terminal){
 if(!s||!id||id>s->last||(terminal!=AF_DONE&&terminal!=AF_DROPPED))return AF_INVALID;
 AfRecord *r=&s->item[id-1];if(epoch!=s->epoch||r->epoch!=epoch||r->state==AF_UNCERTAIN)return AF_STALE;
 if(r->state!=AF_ACCEPTED)return r->state==terminal?AF_OK:AF_CONFLICT;
 r->state=terminal;return AF_OK;
}
int af_ledger_restart(AfLedger *s,unsigned peer_quiet,unsigned device_quiet){
 if(!s||!s->epoch)return AF_INVALID;
 if(!peer_quiet||!device_quiet)return AF_BUSY;
 if(s->epoch==UINT32_MAX)return AF_EXHAUSTED;
 for(unsigned i=0;i<s->last;i++)if(s->item[i].state==AF_ACCEPTED)s->item[i].state=AF_UNCERTAIN;
 s->epoch++;return AF_OK;
}
