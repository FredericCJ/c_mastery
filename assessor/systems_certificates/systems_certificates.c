#include "systems_certificates.h"
static int walk(const SCTree *t,int id,int parent,int64_t lo,int64_t hi,uint64_t *seen,unsigned *height){
 unsigned left,right;const SCNode *n;
 if(id==-1){*height=0;return 1;}
 if(id<0||id>=64||(*seen&(UINT64_C(1)<<(unsigned)id)))return 0;
 n=&t->nodes[id];if(!n->live||n->parent!=parent||n->key<=lo||n->key>=hi)return 0;
 *seen|=UINT64_C(1)<<(unsigned)id;
 if(!walk(t,n->left,id,lo,n->key,seen,&left)||!walk(t,n->right,id,n->key,hi,seen,&right))return 0;
 *height=1+(left>right?left:right);
#ifndef SC_MUTATE_TREE
 if(n->height!=*height)return 0;
#endif
 if(t->balanced&&(left>right+1||right>left+1))return 0;
 return 1;
}
SCStatus sc_tree_check(const SCTree *t,unsigned *height){
 uint64_t seen=0,live=0;unsigned i,n=0,h;
 if(!t||!height||t->count>64||t->balanced>1)return SC_INVALID;
 for(i=0;i<64;i++)if(t->nodes[i].live){live|=UINT64_C(1)<<i;n++;}
 if(n!=t->count||!walk(t,t->root,-1,INT64_MIN,INT64_MAX,&seen,&h)||seen!=live)return SC_INVALID;
 *height=h;return SC_OK;
}
SCStatus sc_unique_insert(void *context,SCContains contains,SCInsert insert,int32_t key){
 int present,result;if(!context||!contains||!insert)return SC_INVALID;present=contains(context,key);if(present<0||present>1)return SC_INVALID;
#ifndef SC_MUTATE_TREE2
 if(present)return SC_DUPLICATE;
#endif
 result=insert(context,key);return result==0?SC_OK:result==2?SC_FULL:SC_INVALID;
}
static int equal_at(const uint8_t *t,const uint8_t *p,size_t position,size_t n){size_t i;for(i=0;i<n;i++)if(t[position+i]!=p[i])return 0;return 1;}
SCStatus sc_matches(const uint8_t *t,size_t n,const uint8_t *p,size_t m,uint8_t *positions,size_t capacity,size_t *count){
 size_t i,total=0,written=0;
 if(!count||(!t&&n)||(!p&&m)||n>64||m>16||(!positions&&capacity))return SC_INVALID;
 if(m<=n)for(i=0;i<=n-m;i++)if(equal_at(t,p,i,m)){
  total++;
#ifdef SC_MUTATE_MATCHES
  if(m)i+=m-1;
#endif
#ifdef SC_MUTATE_MATCHES2
  if(!m)break;
#endif
 }
 if(total>capacity)return SC_FULL;
 if(m<=n)for(i=0;i<=n-m;i++)if(equal_at(t,p,i,m)){
  positions[written++]=(uint8_t)i;
#ifdef SC_MUTATE_MATCHES
  if(m)i+=m-1;
#endif
#ifdef SC_MUTATE_MATCHES2
  if(!m)break;
#endif
 }
 *count=written;return SC_OK;
}
SCStatus sc_clock(SCMachine *m,const SCInstruction *program,size_t length,SCControl *control){
 SCMachine next;SCControl c={0};SCInstruction op;
 if(!m||!control||!program||!length||length>16||m->halted>1)return SC_INVALID;
 if(m->halted)return SC_DONE;
 if(m->pc>=length||m->commits==UINT32_MAX)return SC_INVALID;
 op=program[m->pc];next=*m;c.pc_before=m->pc;next.pc++;
#ifdef SC_MUTATE_CLOCK2
 m->pc++;
#endif
 switch(op.operation){
 case SC_LI:if(op.a>3||op.c)return SC_INVALID;next.reg[op.a]=op.b;c.reg_write=(uint8_t)(1u<<op.a);c.alu=op.b;break;
 case SC_SUB:
  if(op.a>3||op.b>3||op.c>3)return SC_INVALID;
#ifdef SC_MUTATE_CLOCK
  next.reg[op.a]=0;
  c.alu=(uint8_t)((unsigned)next.reg[op.b]+256u-next.reg[op.c]);
#else
  c.alu=(uint8_t)((unsigned)m->reg[op.b]+256u-m->reg[op.c]);
#endif
  next.reg[op.a]=c.alu;c.reg_write=(uint8_t)(1u<<op.a);break;
 case SC_BNZ:if(op.a>3||op.b>=length||op.c)return SC_INVALID;if(m->reg[op.a]){next.pc=op.b;c.branch_taken=1;}break;
 case SC_ST:if(op.a>3||op.b>15||op.c)return SC_INVALID;next.memory[op.b]=m->reg[op.a];c.mem_write=1;c.alu=m->reg[op.a];break;
 case SC_LD:if(op.a>3||op.b>15||op.c)return SC_INVALID;next.reg[op.a]=m->memory[op.b];c.reg_write=(uint8_t)(1u<<op.a);c.alu=next.reg[op.a];break;
 case SC_HALT:if(op.a||op.b||op.c)return SC_INVALID;next.halted=1;next.pc=m->pc;break;
 default:return SC_INVALID;
 }
 if(!next.halted&&next.pc>=length)return SC_INVALID;
 next.commits++;c.pc_after=next.pc;*m=next;*control=c;return SC_OK;
}
SCStatus sc_run(SCMachine *m,const SCInstruction *program,size_t length,unsigned budget,unsigned *executed){
 unsigned n=0;SCControl c;SCStatus status;
 if(!m||!executed||!program||!length||length>16||m->halted>1||(!m->halted&&m->pc>=length))return SC_INVALID;
 while(n<budget){status=sc_clock(m,program,length,&c);if(status!=SC_OK){*executed=n;return status;}n++;if(m->halted){*executed=n;return SC_DONE;}}
 *executed=n;return m->halted?SC_DONE:SC_BUDGET;
}
