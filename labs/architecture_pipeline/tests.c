#include "pipeline.h"
#define CHECK(x) do{if(!(x)){ap_failure(#x,__LINE__);return 41;}}while(0)
static int name(const char*a,const char*b){while(*a && *a==*b){++a;++b;}return *a==*b;}
static int equal(const void*a,const void*b,size_t n){const unsigned char*x=a,*y=b;for(size_t i=0;i<n;++i)if(x[i]!=y[i])return 0;return 1;}
static struct ap_instruction program[]={
 {S_ALU,1,0,0,0,0},{S_ALU,2,1,0,0,0},{S_LOAD,3,2,0,0,0},{S_ALU,4,3,0,0,0},
 {S_BRANCH,0,4,0,6,1},{S_ALU,5,4,0,0,0},{S_ALU,6,4,0,0,0}};
static void emit_schedule(unsigned key,const struct ap_schedule*s){
 int head[]={(int)key,(int)s->cycles,(int)s->instances,(int)s->retired_count,(int)s->flushed_count,(int)s->data_stalls,(int)s->structural_stalls,(int)s->port_stalls};ap_emit("head",head,8);
 for(unsigned i=0;i<s->instances;++i){int row[]={(int)key,(int)i,(int)s->pc[i],(int)s->flush_cycle[i],(int)s->resolve_cycle[i]};ap_emit("instance",row,5);}
 for(unsigned c=0;c<s->cycles;++c){int row[]={(int)key,(int)c+1,s->slots[c][0],s->slots[c][1],s->slots[c][2],s->slots[c][3],s->slots[c][4]};ap_emit("cycle",row,7);}
 for(unsigned i=0;i<s->retired_count;++i){int row[]={(int)key,(int)i,(int)s->retired[i],(int)s->retire_cycle[i]};ap_emit("retire",row,4);}
}
static int schedule(void){
 static struct ap_schedule s;struct ap_config c={1,1,1,0,0};CHECK(ap_schedule(program,7,c,&s)==AP_OK && s.cycles==13 && s.retired_count==6 && s.flushed_count==2);CHECK(s.slots[5][3]==2 && s.slots[6][2]==3);CHECK(s.flush_cycle[5]==8 && s.flush_cycle[6]==8 && s.pc[7]==6 && s.slots[8][0]==7);
 for(unsigned i=0;i<6;++i)CHECK(s.pc[s.retired[i]]==(i<5?i:6));
 emit_schedule(100,&s);
 struct s_instruction prior[4];for(unsigned i=0;i<4;++i)prior[i]=(struct s_instruction){program[i].op,program[i].dst,program[i].left,program[i].right,0};unsigned issue[16]={0},cycles;
 for(unsigned forwarding=0;forwarding<2;++forwarding){c.forwarding=forwarding;CHECK(ap_schedule(program,4,c,&s)==AP_OK);CHECK(s_pipeline(prior,4,forwarding,issue,&cycles)==S_OK && cycles==s.cycles && cycles==(forwarding?9u:14u));emit_schedule(101+forwarding,&s);}
 c.forwarding=0;c.write_first=0;CHECK(ap_schedule(program,4,c,&s)==AP_OK && s.cycles==17);emit_schedule(103,&s);
 c=(struct ap_config){1,2,1,0,0};CHECK(ap_schedule(program,7,c,&s)==AP_OK && s.cycles==14);emit_schedule(104,&s);
 c=(struct ap_config){1,1,1,1,0};CHECK(ap_schedule(program,7,c,&s)==AP_OK && s.cycles==14 && s.flushed_count==1 && s.resolve_cycle[4]==9);emit_schedule(105,&s);
 return 0;
}
static int matrix(void){
 static struct ap_schedule s;for(unsigned f=0;f<2;++f)for(unsigned mem=1;mem<=2;++mem)for(unsigned w=0;w<2;++w)for(unsigned b=0;b<2;++b)for(unsigned u=0;u<2;++u){struct ap_config c={f,mem,w,b,u};unsigned key=f*16+(mem-1)*8+w*4+b*2+u;CHECK(ap_schedule(program,7,c,&s)==AP_OK && s.retired_count==6);emit_schedule(key,&s);}
 struct ap_instruction independent[]={{S_LOAD,1,0,0,0,0},{S_ALU,2,0,0,0,0},{S_ALU,3,0,0,0,0},{S_ALU,4,0,0,0,0}};struct ap_config c={1,1,1,0,1};CHECK(ap_schedule(independent,4,c,&s)==AP_OK && s.port_stalls==1 && s.cycles==9);emit_schedule(200,&s);return 0;
}
static int boundaries(void){
 static struct ap_schedule s,old;struct ap_config c={1,1,1,0,0};CHECK(ap_schedule(0,0,c,&s)==AP_OK && !s.cycles && !s.retired_count);s.cycles=999;old=s;struct ap_instruction p[]={ {S_LOAD,1,0,0,0,0},{S_ALU,2,8,0,0,0} };CHECK(ap_schedule(p,2,c,&s)==AP_INVALID && equal(&s,&old,sizeof(s)));p[1]=(struct ap_instruction){S_BRANCH,0,0,0,0,1};CHECK(ap_schedule(p,2,c,&s)==AP_INVALID && equal(&s,&old,sizeof(s)));c.memory_cycles=0;CHECK(ap_schedule(program,7,c,&s)==AP_INVALID && equal(&s,&old,sizeof(s)));c=(struct ap_config){1,1,1,0,0};p[0]=(struct ap_instruction){S_LOAD,0,0,0,0,0};p[1]=(struct ap_instruction){S_ALU,1,0,0,0,0};CHECK(ap_schedule(p,2,c,&s)==AP_OK && s.cycles==6 && !s.data_stalls);int row[]={(int)s.cycles,(int)s.data_stalls};ap_emit("boundaries",row,2);return 0;
}
static int ooo(void){
 const struct ap_instruction cases[4][4]={
  {{S_LOAD,1,0,0,0,0},{S_ALU,2,1,0,0,0},{S_ALU,3,0,0,0,0},{S_ALU,4,2,3,0,0}},
  {{S_LOAD,1,0,0,0,0},{S_ALU,2,1,0,0,0},{S_ALU,1,0,0,0,0},{S_ALU,3,1,0,0,0}},
  {{S_LOAD,0,0,0,0,0},{S_ALU,1,0,0,0,0},{S_LOAD,2,0,0,0,0},{S_ALU,3,1,2,0,0}},
  {{S_ALU,1,6,7,0,0},{S_LOAD,2,1,0,0,0},{S_ALU,3,1,2,0,0},{S_ALU,1,3,7,0,0}}};
 uint8_t initial[8]={0,0,0,0,0,0,250,10},memory[16];for(unsigned i=0;i<16;++i)memory[i]=(uint8_t)(37*i+5);
 for(unsigned kind=0;kind<4;++kind)for(unsigned latency=1;latency<=4;++latency){struct ap_ooo r;CHECK(ap_ooo(cases[kind],4,initial,memory,latency,&r)==AP_OK);uint8_t sequential[8];for(unsigned i=0;i<8;++i)sequential[i]=initial[i];for(unsigned i=0;i<4;++i){struct ap_instruction op=cases[kind][i];unsigned sum=sequential[op.left]+sequential[op.right];uint8_t value=op.op==S_LOAD?memory[sum&15]:(uint8_t)sum;if(op.dst)sequential[op.dst]=value;}CHECK(equal(r.registers,sequential,sizeof(sequential)));if(!kind && latency==3)CHECK(r.issue[0]==1 && r.issue[2]==2 && r.issue[1]==4);
  for(unsigned i=0;i<4;++i){int row[]={(int)kind,(int)latency,(int)i,(int)r.issue[i],(int)r.complete[i],(int)r.retire[i],r.value[i],r.left_tag[i],r.right_tag[i]};ap_emit("ooo",row,9);}int row[11]={(int)kind,(int)latency,(int)r.cycles};for(unsigned i=0;i<8;++i)row[i+3]=r.registers[i];ap_emit("ooo_final",row,11);
 }
 struct ap_ooo r={0},old;r.cycles=999;old=r;CHECK(ap_ooo(program,7,initial,memory,2,&r)==AP_INVALID && equal(&r,&old,sizeof(r)));CHECK(ap_ooo(0,0,initial,memory,2,&r)==AP_OK && !r.cycles && equal(r.registers,initial,8));return 0;
}
int ap_test(const char*s){if(name(s,"schedule"))return schedule();if(name(s,"matrix"))return matrix();if(name(s,"boundaries"))return boundaries();if(name(s,"ooo"))return ooo();return 42;}
