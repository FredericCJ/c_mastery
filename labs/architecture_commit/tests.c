#include "commit.h"
#define CHECK(x) do{if(!(x)){at_failure(#x,__LINE__);return 41;}}while(0)
static int name(const char*a,const char*b){while(*a && *a==*b){++a;++b;}return *a==*b;}
static int equal(const void*a,const void*b,size_t n){const unsigned char*x=a,*y=b;for(size_t i=0;i<n;++i)if(x[i]!=y[i])return 0;return 1;}
static const struct sc_instruction loop[]={{SC_LI,1,3,0},{SC_LI,2,1,0},{SC_SUB,1,1,2},{SC_BNZ,1,2,0},{SC_ST,1,5,0},{SC_LD,3,5,0},{SC_HALT,0,0,0}};
static int baseline(void){
 {struct sc_machine m={0},old;struct sc_control control={0},old_control;struct sc_instruction sub[]={{SC_SUB,0,0,1},{SC_HALT,0,0,0}};m.reg[0]=9;m.reg[1]=3;CHECK(sc_clock(&m,sub,2,&control)==SC_OK && m.reg[0]==6);m=(struct sc_machine){0};sub[0]=(struct sc_instruction){SC_LD,0,16,0};old=m;old_control=control;CHECK(sc_clock(&m,sub,2,&control)==SC_INVALID && at_same(&m,&old) && equal(&control,&old_control,sizeof(control)));}
 struct sc_machine m={0};struct sc_control control={0};unsigned pc[]={1,2,3,2,3,2,3,4,5,6,6},r1[]={3,3,2,2,1,1,0,0,0,0,0};
 for(unsigned i=0;i<11;++i){CHECK(sc_clock(&m,loop,7,&control)==SC_OK && m.pc==pc[i] && m.reg[1]==r1[i]);unsigned row[]={i,control.pc_before,control.pc_after,control.reg_write,control.mem_write,control.branch_taken,control.alu,m.reg[0],m.reg[1],m.reg[2],m.reg[3],m.commits,m.halted};at_emit("baseline",row,13);}
 struct sc_machine old=m;struct sc_control old_control=control;CHECK(sc_clock(&m,loop,7,&control)==SC_DONE && at_same(&m,&old) && equal(&control,&old_control,sizeof(control)));
 m=(struct sc_machine){0};unsigned count=99;CHECK(sc_run(&m,loop,7,4,&count)==SC_BUDGET && count==4 && m.commits==4);CHECK(sc_run(&m,loop,7,7,&count)==SC_DONE && count==7 && at_same(&m,&old));
 struct sc_instruction bad[]={{SC_LI,1,3,0},{SC_LI,2,1,0},{99,0,0,0}};m=(struct sc_machine){0};CHECK(sc_run(&m,bad,3,9,&count)==SC_INVALID && count==2 && m.commits==2 && m.pc==2 && m.reg[1]==3 && m.reg[2]==1);
 unsigned progress[]={count,m.commits,m.pc,m.reg[1],m.reg[2]};at_emit("progress",progress,5);
 return 0;
}
static int input(void){
 struct sc_machine initial={0};struct at_machine m;struct at_trace trace;CHECK(at_init(&m,&initial,0)==AT_OK);for(unsigned i=0;i<3;++i)CHECK(at_begin(&m,loop,7,&trace)==AT_OK);CHECK(m.state.pc==3 && m.state.reg[1]==2);CHECK(at_input(&m,1,0)==AT_OK);CHECK(at_begin(&m,loop,7,&trace)==AT_OK && m.state.pc==4 && !trace.wires.branch_taken);unsigned row[]={m.state.pc,m.state.reg[1],m.state.commits,m.ticks};at_emit("input",row,4);
 struct at_machine old=m;CHECK(at_input(&m,4,0)==AT_INVALID && equal(&m,&old,sizeof(m)));CHECK(at_input(&m,0,256)==AT_INVALID && equal(&m,&old,sizeof(m)));return 0;
}
static int clocked(void){
 struct sc_machine initial={0};struct at_machine m;struct at_trace trace={0};CHECK(at_init(&m,&initial,0)==AT_OK);
 for(unsigned cycle=0;cycle<12;++cycle){enum at_status status=m.pending?at_poll(&m,1,0,&trace):at_begin(&m,loop,7,&trace);CHECK(status==AT_OK || status==AT_PENDING);unsigned row[]={cycle,trace.phase,trace.committed,trace.left,trace.right,trace.wires.pc_before,trace.wires.pc_after,trace.wires.reg_write,trace.wires.mem_write,trace.wires.branch_taken,trace.wires.alu,m.state.pc,m.state.commits,m.state.halted,m.zero,m.ticks};at_emit("clocked",row,16);}
 CHECK(m.state.halted && m.state.commits==11 && m.ticks==12);struct at_machine old=m;struct at_trace old_trace=trace;CHECK(at_begin(&m,loop,7,&trace)==AT_DONE && equal(&m,&old,sizeof(m)) && equal(&trace,&old_trace,sizeof(trace)));
 CHECK(at_init(&m,&initial,0)==AT_OK);struct sc_instruction invalid[]={{SC_LD,0,16,0},{SC_HALT,0,0,0}};old=m;CHECK(at_begin(&m,invalid,2,&trace)==AT_INVALID && equal(&m,&old,sizeof(m)) && equal(&trace,&old_trace,sizeof(trace)));return 0;
}
static int status(void){
 unsigned zeros=0;struct sc_instruction sub[]={{SC_SUB,0,0,1},{SC_HALT,0,0,0}};
 for(unsigned a=0;a<256;++a)for(unsigned b=0;b<256;++b){struct sc_machine initial={0};initial.reg[0]=(uint8_t)a;initial.reg[1]=(uint8_t)b;struct at_machine m;struct at_trace trace;CHECK(at_init(&m,&initial,0)==AT_OK);CHECK(at_begin(&m,sub,2,&trace)==AT_OK && m.zero==(a==b));unsigned native=(aa_isa_compare(a,b)>>4)&1;CHECK(native==m.zero);zeros+=m.zero;if(a==b || a==0 || b==0){unsigned row[]={a,b,m.state.reg[0],m.zero,native};at_emit("status",row,5);}CHECK(at_begin(&m,sub,2,&trace)==AT_OK && m.zero==(a==b));}
 unsigned row[]={65536,zeros};at_emit("status_count",row,2);CHECK(zeros==256);return 0;
}
static int load(void){
 struct sc_instruction code[]={{SC_LD,1,5,0},{SC_HALT,0,0,0}};
 for(unsigned stalls=0;stalls<4;++stalls)for(unsigned cancel=0;cancel<2;++cancel){struct sc_machine initial={0};initial.reg[1]=99;initial.memory[5]=42;struct at_machine m;struct at_trace trace;CHECK(at_init(&m,&initial,1)==AT_OK);CHECK(at_begin(&m,code,2,&trace)==AT_PENDING && at_same(&m.state,&initial) && trace.wires.alu==42 && trace.wires.reg_write==2 && !trace.committed);
  for(unsigned i=0;i<stalls;++i)CHECK(at_poll(&m,0,0,&trace)==AT_PENDING && at_same(&m.state,&initial) && !trace.committed);
  struct at_machine old=m;CHECK(at_input(&m,0,7)==AT_INVALID && equal(&m,&old,sizeof(m)));CHECK(at_poll(&m,1,cancel,&trace)==(cancel?AT_CANCELLED:AT_OK));CHECK(cancel?at_same(&m.state,&initial):(m.state.reg[1]==42 && m.state.commits==1 && m.state.pc==1));CHECK(!m.pending && m.pending_state.commits==0 && m.pending_control.reg_write==0);old=m;struct at_trace old_trace=trace;CHECK(at_poll(&m,1,1,&trace)==AT_NO_PENDING && equal(&m,&old,sizeof(m)) && equal(&trace,&old_trace,sizeof(trace)));unsigned row[]={stalls,cancel,m.state.reg[1],m.state.commits,m.ticks,m.zero};at_emit("load",row,6);
 }
 struct sc_machine initial={0};struct at_machine m;struct at_trace trace={0};CHECK(at_init(&m,&initial,0)==AT_OK);m.ticks=UINT32_MAX;struct at_machine old=m;CHECK(at_begin(&m,code,2,&trace)==AT_INVALID && equal(&m,&old,sizeof(m)));return 0;
}
static int speculation(void){
 for(unsigned condition=0;condition<2;++condition)for(unsigned prediction=1;prediction<=2;++prediction)for(unsigned op1=0;op1<3;++op1)for(unsigned op2=0;op2<3;++op2){
  struct sc_instruction code[]={{SC_BNZ,0,2,0},{SC_LI,1,77,0},{SC_LI,1,88,0},{SC_HALT,0,0,0}};unsigned ops[]={SC_LI,SC_ST,99};code[1].operation=(uint8_t)ops[op1];code[2].operation=(uint8_t)ops[op2];if(op1==1)code[1].b=5;if(op2==1)code[2].b=6;
  struct sc_machine initial={0};initial.reg[0]=(uint8_t)condition;initial.reg[1]=9;initial.memory[5]=55;initial.memory[6]=66;struct at_machine m;struct at_trace trace;struct at_spec spec={0};CHECK(at_init(&m,&initial,0)==AT_OK);CHECK(at_predict(&m,code,4,prediction,&spec)==AT_OK && at_same(&m.state,&initial));struct sc_machine branch=initial;struct sc_control control;CHECK(sc_clock(&branch,code,4,&control)==SC_OK);
  unsigned selected=condition?2:1,kind=condition?op2:op1;enum at_status expected=prediction!=selected?AT_SQUASHED:kind==2?AT_FAULT:AT_OK;CHECK(at_resolve(&m,&spec,&trace)==expected && at_same(&m.state,&branch));
  if(expected==AT_OK){struct sc_machine final=branch;CHECK(sc_clock(&final,code,4,&control)==SC_OK);CHECK(at_retire(&m,&spec,&trace)==AT_OK && at_same(&m.state,&final));}
  CHECK(!spec.active);unsigned row[]={condition,prediction,op1,op2,expected,m.state.pc,m.state.reg[1],m.state.memory[5],m.state.memory[6],m.state.commits,m.ticks};at_emit("spec",row,11);
 }
 struct sc_machine initial={0};struct sc_instruction code[]={{SC_BNZ,0,2,0},{SC_LI,1,7,0},{SC_HALT,0,0,0}};struct at_machine m;struct at_trace trace={0};struct at_spec spec={0};CHECK(at_init(&m,&initial,0)==AT_OK);CHECK(at_predict(&m,code,3,1,&spec)==AT_OK);CHECK(at_input(&m,0,1)==AT_OK);struct at_machine old=m;struct at_spec old_spec=spec;CHECK(at_resolve(&m,&spec,&trace)==AT_STALE && equal(&m,&old,sizeof(m)) && equal(&spec,&old_spec,sizeof(spec)));CHECK(at_discard(&spec)==AT_OK && !spec.active);
 CHECK(at_input(&m,0,0)==AT_OK);CHECK(at_predict(&m,code,3,1,&spec)==AT_OK);CHECK(at_resolve(&m,&spec,&trace)==AT_OK && spec.active==2);CHECK(at_input(&m,2,1)==AT_OK);old=m;CHECK(at_retire(&m,&spec,&trace)==AT_STALE && equal(&m,&old,sizeof(m)));CHECK(at_discard(&spec)==AT_OK);return 0;
}
int at_test(const char*s){if(name(s,"baseline"))return baseline();if(name(s,"clocked"))return clocked();if(name(s,"input"))return input();if(name(s,"status"))return status();if(name(s,"load"))return load();if(name(s,"speculation"))return speculation();return 42;}
