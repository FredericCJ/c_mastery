#include "scope.h"
#include <limits.h>
#ifndef OCS_MUTATION
#define OCS_MUTATION 0
#endif
struct ow_context;
struct ow_entry {struct ow_context *owner;unsigned id;};
struct ow_context {const struct ocs_plan *plan;unsigned *effects;struct ow_scope scope;struct ocs_report report;struct ow_entry entries[8];};
static void cleanup(void *p){struct ow_entry *e=p;struct ocs_report *r=&e->owner->report;unsigned i=r->count++;r->trace[i]=e->id;r->seen_error[i]=(unsigned)r->converted;}
static er_result body(er_frame *frame,void *p){
 struct ow_context *c=p;
 for(unsigned i=0;i<c->plan->count;++i){
  if(c->plan->fail_before==i+1){(void)er_raise(frame,ER_IO);return er_error(ER_CONTRACT,i);}
  ++*c->effects;c->entries[i]=(struct ow_entry){c,i+1};
  if(ow_defer(&c->scope,cleanup,&c->entries[i],c->plan->policy[i])!=OW_OK)return er_error(ER_CONTRACT,i);
  ++c->report.registered;
 }
 if(c->plan->raised!=ER_OK){(void)er_raise(frame,c->plan->raised);return er_error(ER_CONTRACT,c->plan->count);}
 return er_ok(c->report.registered);
}
enum ow_status ocs_run(const struct ocs_plan *p,unsigned *effects,struct ocs_report *out){
 if(!p||!effects||!out||p->count>8||p->fail_before>p->count||p->raised<ER_OK||p->raised>ER_CONTRACT||*effects>UINT_MAX-p->count)return OW_INVALID;
 for(unsigned i=0;i<p->count;++i)if(p->policy[i]<OW_ALWAYS||p->policy[i]>OW_ON_SUCCESS)return OW_INVALID;
 struct ow_context c={0};c.plan=p;c.effects=effects;er_frame frame={0};er_result r=er_run(&frame,body,&c);c.report.original=r.tag;
 if(OCS_MUTATION!=1)c.report.converted=r.tag==ER_OK?ER_OK:ER_CONTRACT;
 if(ow_finish(&c.scope,r.tag==ER_OK)!=OW_OK)return OW_INVALID;
 if(OCS_MUTATION==1)c.report.converted=r.tag==ER_OK?ER_OK:ER_CONTRACT;
 *out=c.report;return OW_OK;
}
static int live(struct ocs_ticket t){return t.owner&&t.owner->active&&(OCS_MUTATION==4||t.generation==t.owner->generation);}
enum ow_status ocs_begin(struct ocs_recovery *r,struct ocs_ticket *out){
 if(!r||!out)return OW_INVALID;
 if(r->active||r->count||r->finishing)return OW_BUSY;
 if(r->generation==UINT32_MAX)return OW_RETIRED;
 ++r->generation;r->active=1;r->sealed=0;r->errors=0;*out=(struct ocs_ticket){r,r->generation};return OW_OK;
}
enum ow_status ocs_push(struct ocs_ticket t,ocs_closer_fn close,void *context,unsigned id){
 if(!live(t))return OW_STALE;
 struct ocs_recovery *r=t.owner;if(r->finishing||r->sealed)return OW_BUSY;
 if(!close)return OW_INVALID;
 if(r->count==8)return OW_FULL;
 for(unsigned i=0;i<r->count;++i)if(r->entry[i].identity==id)return OW_INVALID;
 r->entry[r->count++]=(struct ocs_entry){close,context,id};return OW_OK;
}
enum ow_status ocs_recover(struct ocs_ticket t,struct ocs_recovered *out){
 if(!out)return OW_INVALID;
 if(!live(t))return OW_STALE;
 struct ocs_recovery *r=t.owner;if(r->finishing)return OW_BUSY;
 r->finishing=1;r->sealed=1;
 while(r->count){
  struct ocs_entry e=r->entry[r->count-1];enum ocs_close result=e.close(e.context,e.identity);
  if(result==OCS_RETAINED&&OCS_MUTATION!=2)break;
  if(result==OCS_RELEASED_ERROR){++r->errors;if(OCS_MUTATION==3)break;}
  --r->count;
 }
 r->finishing=0;if(!r->count||OCS_MUTATION==5)r->active=0;
 if(OCS_MUTATION==6&&r->generation<UINT32_MAX)++r->generation;
 *out=(struct ocs_recovered){r->count,r->errors,r->count!=0};return r->count?OW_BUSY:OW_OK;
}
