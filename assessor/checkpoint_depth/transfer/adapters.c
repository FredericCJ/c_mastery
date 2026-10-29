#include "adapters.h"
#ifndef CPX_MUTANT
#define CPX_MUTANT 0
#endif
enum ow_status cpx_owner_close(struct cpx_owner *o,int (*unregister)(void *,uint64_t),void *context){
 if(!o || !unregister || !o->cookie)return OW_INVALID;
 if(!o->owner.count)return OW_OK;
 if(o->closing)return OW_BUSY;
 o->closing=1;++o->requests;
 if(!unregister(context,o->cookie) && CPX_MUTANT!=3)return OW_BUSY;
 return ow_destroy(&o->owner);
}
enum ow_status cpx_owner_ack(struct cpx_owner *o,uint64_t cookie,int quiescent){
 if(!o || !o->owner.count || !o->closing || cookie!=o->cookie)return OW_STALE;
 return quiescent?ow_destroy(&o->owner):OW_BUSY;
}
struct cpx_sink { cpw_write_fn write;void *context;struct cpx_progress *output; };
static er_progress sink(void *context,const unsigned char *bytes,size_t n){
 struct cpx_sink *s=context;++s->output->callbacks;struct cpw_result r=s->write(s->context,bytes,n);
 if(r.kind==CPW_PROGRESS && r.accepted && r.accepted<=n && !r.cause)return (er_progress){r.accepted,ER_OK};
 if(r.kind==CPW_RETRY && !r.accepted)return (er_progress){0,ER_AGAIN};
 int cause=r.kind==CPW_ERROR && !r.accepted && r.cause>0?r.cause:4096;
 if(!s->output->error.primary)s->output->error.primary=cause;
 return (er_progress){0,ER_IO};
}
er_code cpx_progress_step(struct cpx_progress *p,const unsigned char *bytes,size_t n,cpw_write_fn write,void *context){
 if(!p || !write || (!bytes && n) || p->progress.next>n)return ER_INVALID;
 if(p->progress.code==ER_IO)return ER_IO;
 struct cpx_sink state={write,context,p};p->progress=er_transfer(bytes,n,p->progress.next,sink,&state);
 p->error.accepted=p->progress.next;return p->progress.code;
}
void cpx_progress_cleanup(struct cpx_progress *p,int cause){
 if(p && cause>0 && !p->error.cleanup)p->error.cleanup=cause;
 if(p && CPX_MUTANT==4)p->error.primary=cause;
}
