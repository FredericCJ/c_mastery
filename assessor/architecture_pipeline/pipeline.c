#include "pipeline.h"
static int admitted(const struct ap_instruction*p,size_t n){
 if((!p && n) || n>16)return 0;
 for(size_t i=0;i<n;++i){struct ap_instruction v=p[i];if(v.op>S_BRANCH || v.dst>7 || v.left>7 || v.right>7 || v.taken>1)return 0;if(v.op==S_BRANCH){if(v.dst || v.target<=i || v.target>=n)return 0;}else if(v.taken || v.target)return 0;}return 1;
}
static int ready(unsigned reg,const int stages[5],const struct ap_schedule*r,const struct ap_instruction*p,struct ap_config c,unsigned mem_left,unsigned in_id){
 (void)mem_left;
 if(!reg)return 1;
 for(unsigned stage=2;stage<5;++stage)if(stages[stage]>=0){const struct ap_instruction*v=&p[r->pc[stages[stage]]];if(v->op!=S_BRANCH && v->dst==reg){
  if(in_id || !c.forwarding)return stage==4 && c.write_first;
  if(v->op==S_LOAD){
#ifdef AP_MUTANT_LOAD
   return 1;
#else
   return stage==4 || (stage==3 && mem_left==1);
#endif
  }
  return 1;
 }}return 1;
}
enum ap_status ap_schedule(const struct ap_instruction*p,size_t n,struct ap_config c,struct ap_schedule*out){
 if(!out || !admitted(p,n) || c.forwarding>1 || c.memory_cycles<1 || c.memory_cycles>2 || c.write_first>1 || c.branch_in_id>1 || c.unified_port>1)return AP_INVALID;
 struct ap_schedule value={0};for(unsigned i=0;i<128;++i)for(unsigned k=0;k<5;++k)value.slots[i][k]=-1;
 if(!n){*out=value;return AP_OK;}
 int stages[5]={0,-1,-1,-1,-1};unsigned fetch=1,mem_left=0,resolved[64]={0};value.instances=1;value.pc[0]=0;
 for(unsigned cycle=1;cycle<=128;++cycle){
  for(unsigned k=0;k<5;++k)value.slots[cycle-1][k]=stages[k];
  if(stages[4]>=0){unsigned id=(unsigned)stages[4];if(value.flush_cycle[id])return AP_INVALID;value.retired[value.retired_count]=id;value.retire_cycle[value.retired_count++]=cycle;}
  unsigned flush=0,target=0;int branch=c.branch_in_id?stages[1]:stages[2];
  if(branch>=0){struct ap_instruction v=p[value.pc[branch]];if(v.op==S_BRANCH && !resolved[branch]){
   int operands=!c.branch_in_id || (ready(v.left,stages,&value,p,c,mem_left,1) && ready(v.right,stages,&value,p,c,mem_left,1));
   if(operands){resolved[branch]=1;value.resolve_cycle[branch]=cycle;if(v.taken){flush=c.branch_in_id?1:2;target=v.target;}}
  }}
  int next[5]={stages[0],stages[1],stages[2],stages[3],-1};unsigned next_mem=mem_left;
  if(stages[3]>=0){if(!mem_left)return AP_INVALID;if(mem_left==1){next[4]=stages[3];next[3]=-1;next_mem=0;}else next_mem--;}
  if(stages[2]>=0 && next[3]<0){next[3]=stages[2];next[2]=-1;next_mem=p[value.pc[stages[2]]].op==S_LOAD?c.memory_cycles:1;}
  if(flush){
   for(unsigned k=0;k<flush;++k)if(stages[k]>=0){unsigned id=(unsigned)stages[k];value.flush_cycle[id]=cycle;value.flushed_count++;next[k]=-1;}
   fetch=target;
#ifdef AP_MUTANT_SQUASH
   if(flush==2 && stages[1]>=0 && next[2]<0)next[2]=stages[1];
#endif
  }
  if(stages[1]>=0 && flush!=2){struct ap_instruction v=p[value.pc[stages[1]]];unsigned id_operand=c.branch_in_id && v.op==S_BRANCH;int available=ready(v.left,stages,&value,p,c,mem_left,id_operand) && ready(v.right,stages,&value,p,c,mem_left,id_operand);
#ifdef AP_MUTANT_READ_ORDER
   if(!c.forwarding && !c.write_first){struct ap_config wrong=c;wrong.write_first=1;available=ready(v.left,stages,&value,p,wrong,mem_left,id_operand) && ready(v.right,stages,&value,p,wrong,mem_left,id_operand);}
#endif
   if(available && next[2]<0){next[2]=stages[1];next[1]=-1;}else if(!available)value.data_stalls++;else value.structural_stalls++;
  }
  if(next[1]<0 && next[0]>=0){next[1]=next[0];next[0]=-1;}
  if(next[0]<0 && fetch<n){
   unsigned blocked=c.unified_port && next[3]>=0 && p[value.pc[next[3]]].op==S_LOAD;
#ifdef AP_MUTANT_PORT
   blocked=0;
#endif
   if(blocked)value.port_stalls++;
   else{if(value.instances==64)return AP_LIMIT;next[0]=(int)value.instances;value.pc[value.instances++]=fetch++;}
  }
  for(unsigned k=0;k<5;++k)stages[k]=next[k];
  mem_left=next_mem;value.cycles=cycle;
  if(stages[0]<0 && stages[1]<0 && stages[2]<0 && stages[3]<0 && stages[4]<0){*out=value;return AP_OK;}
 }return AP_LIMIT;
}
enum ap_status ap_ooo(const struct ap_instruction*p,size_t n,const uint8_t initial[8],const uint8_t memory[16],unsigned load_cycles,struct ap_ooo*out){
 if(!out || !initial || !memory || !admitted(p,n) || initial[0] || load_cycles<1 || load_cycles>4)return AP_INVALID;
 for(size_t i=0;i<n;++i)if(p[i].op==S_BRANCH)return AP_INVALID;
 struct ap_ooo result={0};int latest[8];for(unsigned i=0;i<8;++i){latest[i]=-1;result.registers[i]=initial[i];}
 for(size_t i=0;i<n;++i){result.left_tag[i]=latest[p[i].left];result.right_tag[i]=latest[p[i].right];if(p[i].dst)latest[p[i].dst]=(int)i;}
 if(!n){*out=result;return AP_OK;}
 unsigned retired=0,memory_until=0;
 for(unsigned cycle=1;cycle<=128;++cycle){
  if(retired<n && result.complete[retired] && result.complete[retired]<cycle){unsigned i=retired;result.retire[i]=cycle;if(p[i].dst)result.registers[p[i].dst]=result.value[i];++retired;}
  if(retired==n){result.cycles=cycle;*out=result;return AP_OK;}
  for(unsigned i=0;i<n;++i)if(!result.issue[i]){int left=result.left_tag[i],right=result.right_tag[i];
#ifdef AP_MUTANT_TAG
   left=-1;right=-1;
#endif
   if((left>=0 && (!result.complete[left] || result.complete[left]>=cycle)) || (right>=0 && (!result.complete[right] || result.complete[right]>=cycle)) || (p[i].op==S_LOAD && memory_until>=cycle))continue;
   unsigned a=left<0?initial[p[i].left]:result.value[left],b=right<0?initial[p[i].right]:result.value[right];result.issue[i]=cycle;result.complete[i]=cycle+(p[i].op==S_LOAD?load_cycles:1)-1;result.value[i]=p[i].op==S_LOAD?memory[(a+b)&15]:(uint8_t)(a+b);if(p[i].op==S_LOAD)memory_until=result.complete[i];break;
  }
 }return AP_LIMIT;
}
