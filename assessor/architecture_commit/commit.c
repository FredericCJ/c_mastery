#include "commit.h"
int at_same(const SCMachine*a,const SCMachine*b){if(!a || !b)return 0;for(unsigned i=0;i<4;++i)if(a->reg[i]!=b->reg[i])return 0;for(unsigned i=0;i<16;++i)if(a->memory[i]!=b->memory[i])return 0;return a->pc==b->pc && a->halted==b->halted && a->commits==b->commits;}
static int valid(const ATMachine*m){return m && m->state.halted<=1 && m->state.pc<16 && m->zero<=1 && m->pending<=1 && m->pending_zero<=1;}
static unsigned zflag(unsigned previous,SCInstruction op,SCControl c){
 if(op.operation!=SC_SUB)return previous;
#ifdef AT_MUTANT_ZERO
 return c.alu!=0;
#else
 return c.alu==0;
#endif
}
static void clear_pending(ATMachine*m){m->pending=0;m->pending_zero=0;m->pending_state=(SCMachine){0};m->pending_control=(SCControl){0};}
enum ATStatus at_init(ATMachine*out,const SCMachine*state,unsigned zero){
 if(!out || !state || state->halted>1 || state->pc>=16 || zero>1)return AT_INVALID;
 ATMachine next={0};next.state=*state;next.zero=zero;*out=next;return AT_OK;
}
enum ATStatus at_input(ATMachine*m,unsigned reg,unsigned value){
 if(!valid(m) || reg>3 || value>255 || m->pending || m->state.halted)return AT_INVALID;
#ifdef AT_MUTANT_INPUT
 (void)value;
#else
 m->state.reg[reg]=(uint8_t)value;
#endif
 return AT_OK;
}
enum ATStatus at_begin(ATMachine*m,const SCInstruction*p,size_t n,ATTrace*out){
 if(!valid(m) || !out || !p || !n || n>16 || m->pending || m->ticks==UINT32_MAX)return AT_INVALID;
 SCMachine next=m->state;SCControl c={0};SCStatus s=sc_clock(&next,p,n,&c);
 if(s==SC_DONE)return AT_DONE;
 if(s!=SC_OK)return AT_INVALID;
 SCInstruction op=p[m->state.pc];ATMachine value=*m;ATTrace trace={0};trace.wires=c;trace.phase=AT_COMMIT;trace.ticks=m->ticks+1;value.ticks++;
 if(op.operation==SC_SUB){trace.left=m->state.reg[op.b];trace.right=m->state.reg[op.c];}
 else if(op.operation==SC_LI)trace.right=op.b;
 else if(op.operation==SC_ST || op.operation==SC_BNZ)trace.left=m->state.reg[op.a];
 else if(op.operation==SC_LD)trace.right=m->state.memory[op.b];
 unsigned zero=zflag(m->zero,op,c);
 if(op.operation==SC_LD){value.pending=1;value.pending_state=next;value.pending_control=c;value.pending_zero=zero;trace.phase=AT_ISSUE;}
 else{value.state=next;value.zero=zero;trace.committed=1;}
 *m=value;*out=trace;return value.pending?AT_PENDING:AT_OK;
}
enum ATStatus at_poll(ATMachine*m,unsigned ready,unsigned cancel,ATTrace*out){
 if(!valid(m) || !out || ready>1 || cancel>1)return AT_INVALID;
 if(!m->pending)return AT_NO_PENDING;
 if(m->ticks==UINT32_MAX)return AT_INVALID;
 ATMachine value=*m;ATTrace trace={0};trace.wires=m->pending_control;trace.right=m->pending_control.alu;trace.ticks=m->ticks+1;value.ticks++;trace.phase=AT_WAIT;enum ATStatus status=AT_PENDING;
#ifdef AT_MUTANT_CANCEL
 if(ready){value.state=m->pending_state;value.zero=m->pending_zero;trace.phase=AT_COMMIT;trace.committed=1;status=AT_OK;clear_pending(&value);}
 else if(cancel){trace.phase=AT_CANCEL;status=AT_CANCELLED;clear_pending(&value);}
#else
 if(cancel){trace.phase=AT_CANCEL;status=AT_CANCELLED;clear_pending(&value);}
 else if(ready){value.state=m->pending_state;value.zero=m->pending_zero;trace.phase=AT_COMMIT;trace.committed=1;status=AT_OK;clear_pending(&value);}
#endif
 *m=value;*out=trace;return status;
}
enum ATStatus at_predict(const ATMachine*m,const SCInstruction*p,size_t n,unsigned prediction,ATSpec*out){
 if(!valid(m) || m->pending || m->state.halted || !out || out->active || !p || !n || n>16 || m->state.pc>=n || prediction>=n || p[m->state.pc].operation!=SC_BNZ)return AT_INVALID;
 ATSpec value={0};value.origin=m->state;value.branch=m->state;
 if(sc_clock(&value.branch,p,n,&value.branch_control)!=SC_OK)return AT_INVALID;
 value.active=1;value.predicted_pc=prediction;value.candidate=value.branch;value.candidate.pc=(uint8_t)prediction;
 value.candidate_valid=sc_clock(&value.candidate,p,n,&value.candidate_control)==SC_OK;
 if(value.candidate_valid)value.candidate_zero=zflag(m->zero,p[prediction],value.candidate_control);
 *out=value;return AT_OK;
}
enum ATStatus at_resolve(ATMachine*m,ATSpec*s,ATTrace*out){
 if(!valid(m) || m->pending || !s || s->active!=1 || !out || m->ticks==UINT32_MAX)return AT_INVALID;
 if(!at_same(&m->state,&s->origin))return AT_STALE;
 ATMachine value=*m;ATSpec spec=*s;ATTrace trace={0};value.state=s->branch;value.ticks++;trace.ticks=value.ticks;trace.wires=s->branch_control;trace.phase=AT_BRANCH;trace.committed=1;enum ATStatus status;
 if(s->predicted_pc!=s->branch.pc){
#ifdef AT_MUTANT_SQUASH
  if(s->candidate_valid)value.state=s->candidate;
#endif
  spec=(ATSpec){0};status=AT_SQUASHED;
 }else if(!s->candidate_valid){spec=(ATSpec){0};status=AT_FAULT;}
 else{spec.active=2;status=AT_OK;}
 *m=value;*s=spec;*out=trace;return status;
}
enum ATStatus at_retire(ATMachine*m,ATSpec*s,ATTrace*out){
 if(!valid(m) || m->pending || !s || s->active!=2 || !s->candidate_valid || !out || m->ticks==UINT32_MAX)return AT_INVALID;
 if(!at_same(&m->state,&s->branch))return AT_STALE;
 ATMachine value=*m;ATTrace trace={0};value.state=s->candidate;value.zero=s->candidate_zero;value.ticks++;trace.wires=s->candidate_control;trace.phase=AT_RETIRE;trace.committed=1;trace.ticks=value.ticks;*m=value;*s=(ATSpec){0};*out=trace;return AT_OK;
}
enum ATStatus at_discard(ATSpec*s){if(!s || s->active>2)return AT_INVALID;*s=(ATSpec){0};return AT_OK;}
