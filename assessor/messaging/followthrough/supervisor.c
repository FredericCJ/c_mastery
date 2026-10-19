#include "supervisor.h"
#ifndef MFS_MUTANT
#define MFS_MUTANT 0
#endif
static void event(MfsReport*r,int value){if(r)r->events[r->count++]=value;}
static void stop(MfsTree*t,unsigned g,unsigned c,MfsReport*r){
  bool live=t->actors[g][c].actor.alive||t->resources[g][c];
  (void)mfa_destroy(&t->actors[g][c]);
  if(t->resources[g][c]){t->close(t->resource_ctx,g,c);t->resources[g][c]=false;}
  if(live)event(r,-(int)(4*g+c+1));
}
static bool start(MfsTree*t,unsigned g,unsigned c,unsigned failures,MfsReport*r){
  unsigned attempt=r?r->attempts++:0;
  bool fail=(r && (failures&(1u<<attempt))) || (c==1&&!t->resources[g][0]);
  if(!fail)fail=!t->open(t->resource_ctx,g,c);
  if(!fail){
    t->resources[g][c]=true;
    if(mfa_restart(&t->actors[g][c])!=MSG_OK){t->close(t->resource_ctx,g,c);t->resources[g][c]=false;fail=true;}
  }
  event(r,(int)(4*g+c+1)+(fail?100:0));return !fail;
}
static void stop_group(MfsTree*t,unsigned g,MfsReport*r){
  for(unsigned c=4;c;c--)stop(t,g,c-1,r);
}
static void stop_all(MfsTree*t,MfsReport*r){
  for(unsigned g=4;g;g--)stop_group(t,g-1,r);
  t->closed=true;
}
static bool quiescent(const MfsTree*t){
  for(unsigned g=0;g<4;g++)for(unsigned c=0;c<4;c++)
    if(t->actors[g][c].actor.stepping)return false;
  return true;
}
MsgStatus mfs_init(MfsTree*t,size_t parent,size_t child,uint64_t window,
                   MfsOpen open,MfsClose close,void*resource_ctx,MfaHandler handler,
                   void*contexts[4][4],MfaDeadLetter dead,void*dead_ctx){
  if(!t||!open||!close||!handler||!dead||!contexts||!window||!parent||parent>4||!child||child>4)return MSG_INVALID;
  *t=(MfsTree){.open=open,.close=close,.resource_ctx=resource_ctx,.child_limit=child,.window=window};
  (void)msg_supervisor_init(&t->parent,parent,window);
  for(unsigned g=0;g<4;g++){
    (void)msg_supervisor_init(&t->children[g],child,window);
    for(unsigned c=0;c<4;c++){
      (void)mfa_init(&t->actors[g][c],handler,contexts[g][c],dead,dead_ctx);
      (void)mfa_destroy(&t->actors[g][c]);
    }
  }
  for(unsigned g=0;g<4;g++)for(unsigned c=0;c<4;c++)if(!start(t,g,c,0,NULL)){
    stop_all(t,NULL);return MSG_FAILED;
  }
  return MSG_OK;
}
MsgStatus mfs_fault(MfsTree*t,unsigned group,unsigned child,uint64_t now,
                    MsgStrategy strategy,unsigned failures,MfsReport*out){
  if(!t||!out||group>3||child>3||(strategy!=MSG_ONE&&strategy!=MSG_ALL&&strategy!=MSG_REST)||failures>255||now<t->last)return MSG_INVALID;
  if(t->closed)return MSG_CLOSED;
  if(!quiescent(t))return MSG_BUSY;
  MsgSupervisor candidate=t->children[group];int actions[8];size_t count=0;
  MsgStatus status=msg_restart_plan(&candidate,now,strategy,child,actions,&count);
  if(status!=MSG_OK&&status!=MSG_FAILED)return status;
  MfsReport report={0};t->last=now;t->children[group]=candidate;
  bool escalate=status==MSG_FAILED;
  for(size_t i=0;i<count;i++){
    int a=actions[i];
    if(a<0)stop(t,group,(unsigned)(-a-1),&report);
    else if(!start(t,group,(unsigned)(a-1),failures,&report)){
      report.rollback=true;escalate=true;
      stop_group(t,group,&report);
      break;
    }
  }
  if(escalate){
    report.parent_used=true;
    candidate=t->parent;
    status=msg_restart_plan(&candidate,now,MSG_ONE,group,actions,&count);
    t->parent=candidate;
    if(status!=MSG_OK){stop_all(t,&report);status=MSG_FAILED;}
    else{
      /* Replacing a child supervisor requires the entire branch quiescent. */
      stop_group(t,group,&report);
      for(unsigned c=0;c<4;c++){
        unsigned target=MFS_MUTANT==2 ? 3-c : c;
        if(!start(t,group,target,failures,&report)){
          report.rollback=true;if(MFS_MUTANT!=1)stop_all(t,&report);status=MSG_FAILED;break;
        }
      }
      if(status==MSG_OK)(void)msg_supervisor_init(&t->children[group],t->child_limit,t->window);
    }
  }
  report.closed=t->closed;*out=report;return status;
}
MsgStatus mfs_destroy(MfsTree*t){
  if(!t)return MSG_INVALID;
  if(!quiescent(t))return MSG_BUSY;
  stop_all(t,NULL);return MSG_OK;
}
