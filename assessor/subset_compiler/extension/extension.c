/* Original bounded compiler extension; separate from learner scaffold. */
#include "extension.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#ifndef CE_MUTATION
#define CE_MUTATION 0
#endif
enum { ID=256,NUM,EQ };
typedef struct {const char*s;size_t n,pos,lo,hi;unsigned tok;uint32_t num;char word[16];CEProgram p;unsigned f,depth;char vars[CE_LOCALS][16];CEDiagnostic*d;} Parser;
static int fail(Parser*p,unsigned why){p->d->begin=p->lo;p->d->end=p->hi;p->d->reason=why;return 0;}
static int alpha(unsigned c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_';}
static int digit(unsigned c){return c>='0'&&c<='9';}
static int next(Parser*p){size_t i=p->pos,k;uint32_t v=0;unsigned base=10,d;for(;;){while(i<p->n&&(p->s[i]==' '||p->s[i]=='\n'||p->s[i]=='\t'||p->s[i]=='\r'))++i;
 if(i+1<p->n&&p->s[i]=='/'&&p->s[i+1]=='/'){i+=2;while(i<p->n&&p->s[i]!='\n')++i;continue;}
 if(i+1<p->n&&p->s[i]=='/'&&p->s[i+1]=='*'){size_t start=i;i+=2;while(i+1<p->n&&(p->s[i]!='*'||p->s[i+1]!='/'))++i;if(i+1>=p->n){p->lo=start;p->hi=p->n;return fail(p,1);}i+=2;continue;}break;}
 p->lo=i;p->word[0]=0;p->num=0;if(i==p->n){p->tok=0;p->hi=p->pos=i;return 1;}
 if(alpha((unsigned char)p->s[i])){k=0;while(i<p->n&&(alpha((unsigned char)p->s[i])||digit((unsigned char)p->s[i]))){if(k<15)p->word[k++]=p->s[i];++i;}p->word[k]=0;p->hi=i;if(i-p->lo>15)return fail(p,2);p->tok=ID;}
 else if(digit((unsigned char)p->s[i])){int bad=0;if(i+1<p->n&&p->s[i]=='0'&&(p->s[i+1]=='x'||p->s[i+1]=='X')){base=16;i+=2;}k=i;while(i<p->n&&(digit((unsigned char)p->s[i])||(base==16&&((p->s[i]>='a'&&p->s[i]<='f')||(p->s[i]>='A'&&p->s[i]<='F'))))){d=digit((unsigned char)p->s[i])?(unsigned)(p->s[i]-'0'):(unsigned)((p->s[i]|32)-'a'+10);if(v>(UINT32_MAX-d)/base)bad=1;else v=v*base+d;++i;}if(i==k)bad=1;if(i<p->n&&(alpha((unsigned char)p->s[i])||digit((unsigned char)p->s[i]))){bad=1;while(i<p->n&&(alpha((unsigned char)p->s[i])||digit((unsigned char)p->s[i])))++i;}p->hi=i;if(bad)return fail(p,3);p->tok=NUM;p->num=v;}
 else {p->tok=(unsigned char)p->s[i++];if(p->tok=='='&&i<p->n&&p->s[i]=='='){p->tok=EQ;++i;}p->hi=i;}
 p->pos=i;return 1;}
static int kw(Parser*p,const char*s){return p->tok==ID&&!strcmp(p->word,s);}
static int take(Parser*p,unsigned t){if(p->tok!=t)return fail(p,4);return next(p);}
static int ins(Parser*p,unsigned op,unsigned arg){if(p->p.count==CE_CODE)return fail(p,5);p->p.code[p->p.count++]=(CEIns){op,arg};return 1;}
static int var(Parser*p,const char*s){unsigned i;for(i=0;i<p->p.fn[p->f].locals;++i)if(!strcmp(p->vars[i],s))return (int)i;return -1;}
static int expr(Parser*,unsigned*);
static int atom(Parser*p,unsigned*t){char name[16];int slot;unsigned i,u;if(++p->depth>32)return fail(p,6);
 if(p->tok==NUM){u=p->num;if(!next(p)||!ins(p,CE_CONST,u))return 0;*t=CE_U32;}
 else if(p->tok=='('){if(!next(p)||!expr(p,t)||!take(p,')'))return 0;}
 else if(p->tok=='&'){if(!next(p)||!kw(p,"a")||!next(p)||!take(p,'[')||!expr(p,&u)||u!=CE_U32||!take(p,']')||!ins(p,CE_ADDR,0))return fail(p,7);*t=CE_PTR;}
 else if(p->tok=='*'){if(!next(p)||!atom(p,&u)||u!=CE_PTR||!ins(p,CE_READ,0))return fail(p,7);*t=CE_U32;}
 else if(p->tok==ID){strcpy(name,p->word);if(!next(p))return 0;
  if(!strcmp(name,"tick")){if(!take(p,'(')||!expr(p,&u)||u!=CE_U32||!take(p,')')||!ins(p,CE_TICK,0))return fail(p,7);*t=CE_U32;}
  else if(!strcmp(name,"i32")||!strcmp(name,"u32")){unsigned want=!strcmp(name,"i32")?CE_I32:CE_U32;if(!take(p,'(')||!expr(p,&u)||u==CE_PTR||!take(p,')'))return fail(p,7);if(u!=want&&!ins(p,want==CE_I32?CE_TO_I32:CE_TO_U32,0))return 0;*t=want;}
  else if(!strcmp(name,"a")){if(!take(p,'[')||!expr(p,&u)||u!=CE_U32||!take(p,']')||!ins(p,CE_ADDR,0)||!ins(p,CE_READ,0))return fail(p,7);*t=CE_U32;}
  else if(p->tok=='('){for(i=0;i<p->f;++i)if(!strcmp(p->p.fn[i].name,name))break;if(i==p->f)return fail(p,8);if(!next(p)||!expr(p,&u)||u!=CE_U32||!take(p,')')||!ins(p,CE_CALL,i))return fail(p,7);*t=CE_U32;}
  else {slot=var(p,name);if(slot<0)return fail(p,8);*t=p->p.fn[p->f].types[slot];if(!ins(p,CE_LOAD,(unsigned)slot))return 0;}}
 else return fail(p,4);--p->depth;return 1;}
static int numeric(Parser*p,unsigned left,unsigned right,unsigned*out){if(left==CE_PTR||right==CE_PTR)return fail(p,7);*out=left==CE_I32&&right==CE_I32?CE_I32:CE_U32;if(*out==CE_U32){if(left==CE_I32&&!ins(p,CE_TO_U32,1))return 0;if(right==CE_I32&&!ins(p,CE_TO_U32,0))return 0;}return 1;}
static int mul(Parser*p,unsigned*t){unsigned u,v;if(!atom(p,t))return 0;while(p->tok=='*'){if(!next(p)||!atom(p,&u)||!numeric(p,*t,u,&v)||!ins(p,v==CE_I32?CE_SMUL:CE_MUL,0))return fail(p,7);*t=v;}return 1;}
static int add(Parser*p,unsigned*t){unsigned op,u,v;if(!mul(p,t))return 0;while(p->tok=='+'||p->tok=='-'){op=p->tok;if(!next(p)||!mul(p,&u))return 0;if(*t==CE_PTR){if(op!='+'||u!=CE_U32||!ins(p,CE_PADD,0))return fail(p,7);}else{if(!numeric(p,*t,u,&v)||!ins(p,v==CE_I32?(op=='+'?CE_SADD:CE_SSUB):(op=='+'?CE_ADD:CE_SUB),0))return 0;*t=v;}}return 1;}
static int expr(Parser*p,unsigned*t){unsigned op,u,v;if(!add(p,t))return 0;while(p->tok=='<'||p->tok==EQ){op=p->tok;if(!next(p)||!add(p,&u)||!numeric(p,*t,u,&v)||!ins(p,v==CE_I32?(op=='<'?CE_SLT:CE_SEQ):(op=='<'?CE_LT:CE_EQ),0))return fail(p,7);*t=CE_U32;}return 1;}
static int block(Parser*p,int declarations){unsigned t,j,k,start,type;int slot;char name[16];if(++p->depth>32||!take(p,'{'))return fail(p,6);
 while(p->tok!='}'){if(!p->tok)return fail(p,4);
 if(kw(p,"let")||kw(p,"ptr")||kw(p,"int")){if(!declarations)return fail(p,9);type=kw(p,"ptr")?CE_PTR:kw(p,"int")?CE_I32:CE_U32;if(!next(p)||p->tok!=ID)return fail(p,4);strcpy(name,p->word);if(!strcmp(name,"a")||!strcmp(name,"x")||var(p,name)>=0||p->p.fn[p->f].locals==CE_LOCALS)return fail(p,8);slot=(int)p->p.fn[p->f].locals;if(!next(p)||!take(p,'=')||!expr(p,&t)||t!=type||!take(p,';'))return fail(p,7);strcpy(p->vars[slot],name);p->p.fn[p->f].types[slot]=type;++p->p.fn[p->f].locals;if(!ins(p,CE_STORE,(unsigned)slot))return 0;continue;}
 declarations=0;
 if(kw(p,"return")){if(!next(p)||!expr(p,&t)||t!=CE_U32||!take(p,';')||!ins(p,CE_RET,0))return fail(p,7);}
 else if(kw(p,"if")){if(!next(p)||!take(p,'(')||!expr(p,&t)||t!=CE_U32||!take(p,')'))return fail(p,7);j=p->p.count;if(!ins(p,CE_JZ,0)||!block(p,0))return 0;k=p->p.count;if(!ins(p,CE_JMP,0))return 0;p->p.code[j].arg=p->p.count;if(kw(p,"else")){if(!next(p)||!block(p,0))return 0;}p->p.code[k].arg=p->p.count;}
 else if(kw(p,"while")){start=p->p.count;if(!next(p)||!take(p,'(')||!expr(p,&t)||t!=CE_U32||!take(p,')'))return fail(p,7);j=p->p.count;if(!ins(p,CE_JZ,0)||!block(p,0)||!ins(p,CE_JMP,start))return 0;p->p.code[j].arg=p->p.count;}
 else if(p->tok=='*'){if(!next(p)||!atom(p,&t)||t!=CE_PTR||!take(p,'=')||!expr(p,&t)||t!=CE_U32||!take(p,';')||!ins(p,CE_WRITE,0))return fail(p,7);}
 else if(p->tok==ID){strcpy(name,p->word);slot=var(p,name);if(!next(p))return 0;if(!strcmp(name,"a")){if(!take(p,'[')||!expr(p,&t)||t!=CE_U32||!take(p,']')||!ins(p,CE_ADDR,0)||!take(p,'=')||!expr(p,&t)||t!=CE_U32||!take(p,';')||!ins(p,CE_WRITE,0))return fail(p,7);}else{if(slot<0||!take(p,'=')||!expr(p,&t)||t!=p->p.fn[p->f].types[slot]||!take(p,';')||!ins(p,CE_STORE,(unsigned)slot))return fail(p,7);}}
 else return fail(p,4);
 }
 --p->depth;return next(p);}
int ce_compile(const char*s,size_t n,CEProgram*out,CEDiagnostic*d){Parser p;unsigned h[CE_CODE],i;CEDiagnostic local;if(!s||!out)return 0;if(!d)d=&local;memset(&p,0,sizeof p);p.s=s;p.n=n;p.d=d;if(!next(&p))return 0;
 while(p.tok){if(p.f==CE_FUNCS||!kw(&p,"fn")||!next(&p)||p.tok!=ID)return fail(&p,4);for(i=0;i<p.f;++i)if(!strcmp(p.p.fn[i].name,p.word))return fail(&p,8);strcpy(p.p.fn[p.f].name,p.word);if(!strcmp(p.word,"a"))return fail(&p,8);p.p.fn[p.f].begin=p.p.count;p.p.fn[p.f].locals=1;p.p.fn[p.f].types[0]=CE_U32;strcpy(p.vars[0],"x");if(!next(&p)||!take(&p,'(')||!kw(&p,"x")||!next(&p)||!take(&p,')')||!block(&p,1))return 0;p.p.fn[p.f].end=p.p.count;++p.f;p.p.functions=p.f;}
 if(!p.f||strcmp(p.p.fn[p.f-1].name,"main")||!ce_verify(&p.p,h))return fail(&p,10);*out=p.p;d->begin=d->end=n;d->reason=0;return 1;}
/* Worklist dataflow uses an exact type stack; any unequal join is rejected.
   Locals and the implicit eight-element array are initialized in every frame.
   No pointer can escape: calls accept/return u32 and functions are acyclic. */
int ce_verify(const CEProgram*p,unsigned heights[CE_CODE]){unsigned f,pc,i,head,tail,q[CE_CODE],seen[CE_CODE]={0},types[CE_CODE][CE_STACK],stack[CE_STACK];if(!p||!heights||!p->functions||p->functions>CE_FUNCS||!p->count||p->count>CE_CODE)return 0;
 for(f=0;f<p->functions;++f){const CEFunc*fn=&p->fn[f];if(fn->begin!=(f?p->fn[f-1].end:0)||fn->end<=fn->begin||fn->end>p->count||!fn->locals||fn->locals>CE_LOCALS||fn->types[0]!=CE_U32)return 0;for(i=0;i<fn->locals;++i)if(fn->types[i]!=CE_U32&&fn->types[i]!=CE_PTR&&fn->types[i]!=CE_I32)return 0;
 for(pc=fn->begin;pc<fn->end;++pc){CEIns a=p->code[pc];if(a.op<CE_CONST||a.op>CE_TICK)return 0;if((a.op==CE_LOAD||a.op==CE_STORE)&&a.arg>=fn->locals)return 0;if((a.op==CE_JZ||a.op==CE_JMP)&&(a.arg<fn->begin||a.arg>=fn->end))return 0;if(a.op==CE_CALL&&a.arg>=f)return 0;}
 head=tail=0;q[tail++]=fn->begin;seen[fn->begin]=1;heights[fn->begin]=0;
 while(head<tail){unsigned sp,targets[2],nt=0,u;CEIns a;pc=q[head++];sp=heights[pc];for(i=0;i<sp;++i)stack[i]=types[pc][i];a=p->code[pc];
 #define POP(t) do{if(!sp||stack[sp-1]!=(t))return 0;--sp;}while(0)
 #define PUSH(t) do{if(sp==CE_STACK)return 0;stack[sp++]=(t);}while(0)
 switch(a.op){case CE_CONST:PUSH(CE_U32);break;case CE_LOAD:PUSH(fn->types[a.arg]);break;case CE_STORE:POP(fn->types[a.arg]);break;
 case CE_ADD:case CE_SUB:case CE_MUL:case CE_LT:case CE_EQ:POP(CE_U32);POP(CE_U32);PUSH(CE_U32);break;
 case CE_TO_I32:if(a.arg)return 0;POP(CE_U32);PUSH(CE_I32);break;case CE_TO_U32:if(a.arg>1||sp<=a.arg||stack[sp-1-a.arg]!=CE_I32)return 0;stack[sp-1-a.arg]=CE_U32;break;case CE_SADD:case CE_SSUB:case CE_SMUL:case CE_SLT:case CE_SEQ:POP(CE_I32);POP(CE_I32);PUSH(a.op==CE_SLT||a.op==CE_SEQ?CE_U32:CE_I32);break;
 case CE_ADDR:POP(CE_U32);PUSH(CE_PTR);break;case CE_PADD:POP(CE_U32);POP(CE_PTR);PUSH(CE_PTR);break;case CE_READ:POP(CE_PTR);PUSH(CE_U32);break;case CE_WRITE:POP(CE_U32);POP(CE_PTR);break;
 case CE_TICK:case CE_CALL:POP(CE_U32);PUSH(CE_U32);break;case CE_JZ:POP(CE_U32);targets[nt++]=a.arg;break;case CE_JMP:targets[nt++]=a.arg;break;
 case CE_RET:POP(CE_U32);if(sp)return 0;break;default:return 0;}
 if(a.op!=CE_RET&&a.op!=CE_JMP){if(pc+1>=fn->end)return 0;targets[nt++]=pc+1;}
 for(u=0;u<nt;++u){unsigned dst=targets[u];if(seen[dst]){if(heights[dst]!=sp)return 0;for(i=0;i<sp;++i)if(types[dst][i]!=stack[i]&&CE_MUTATION!=2)return 0;}else{seen[dst]=1;heights[dst]=sp;for(i=0;i<sp;++i)types[dst][i]=stack[i];q[tail++]=dst;}}
 #undef POP
 #undef PUSH
 }}if(p->fn[p->functions-1].end!=p->count)return 0;for(pc=0;pc<p->count;++pc)if(!seen[pc])heights[pc]=UINT_MAX;return 1;}
static int64_t signed_value(uint32_t x){return x<=INT32_MAX?(int64_t)x:(int64_t)x-INT64_C(4294967296);}
static int execute(const CEProgram*p,unsigned f,uint32_t x,unsigned*fuel,uint32_t*effects,uint32_t*out){uint32_t locals[CE_LOCALS]={0},a[CE_ARRAY]={0},s[CE_STACK],v,b;unsigned pc,sp=0;locals[0]=x;
 for(pc=p->fn[f].begin;pc<p->fn[f].end;){CEIns in=p->code[pc++];if(!*fuel)return 0;--*fuel;switch(in.op){case CE_CONST:s[sp++]=in.arg;break;case CE_LOAD:s[sp++]=locals[in.arg];break;case CE_STORE:locals[in.arg]=s[--sp];break;
 case CE_ADD:case CE_SUB:case CE_MUL:case CE_LT:case CE_EQ:b=s[--sp];v=s[sp-1];s[sp-1]=in.op==CE_ADD?v+b:in.op==CE_SUB?(CE_MUTATION==1?b-v:v-b):in.op==CE_MUL?v*b:in.op==CE_LT?(uint32_t)(v<b):(uint32_t)(v==b);break;
 case CE_TO_I32:if(s[sp-1]>INT32_MAX)return 0;break;case CE_TO_U32:break;case CE_SADD:case CE_SSUB:case CE_SMUL:case CE_SLT:case CE_SEQ:{int64_t y=signed_value(s[--sp]),x=signed_value(s[sp-1]);int64_t r=in.op==CE_SADD?x+y:in.op==CE_SSUB?x-y:in.op==CE_SMUL?x*y:in.op==CE_SLT?x<y:x==y;if(r<INT32_MIN||r>INT32_MAX)return 0;s[sp-1]=(uint32_t)r;break;}
 case CE_ADDR:if(s[sp-1]>CE_ARRAY)return 0;break;case CE_PADD:b=s[--sp];v=s[sp-1];if(v>CE_ARRAY||b>CE_ARRAY-v)return 0;s[sp-1]=v+b;break;
 case CE_READ:v=s[sp-1];if(v>=CE_ARRAY&&CE_MUTATION!=3)return 0;if(v>=CE_ARRAY){s[sp-1]=0;break;}s[sp-1]=a[v];break;
 case CE_WRITE:v=s[--sp];b=s[--sp];if(b>=CE_ARRAY)return 0;a[b]=v;break;case CE_JZ:if(!s[--sp])pc=in.arg;break;case CE_JMP:pc=in.arg;break;
 case CE_TICK:s[sp-1]+=*effects;*effects+=1;break;case CE_CALL:if(!execute(p,in.arg,s[sp-1],fuel,effects,&v))return 0;s[sp-1]=v;break;case CE_RET:*out=s[--sp];return 1;default:return 0;}}return 0;}
int ce_run(const CEProgram*p,uint32_t x,unsigned*fuel,uint32_t*out){unsigned h[CE_CODE];uint32_t v,effects=0;if(!fuel||!out||!ce_verify(p,h)||!execute(p,p->functions-1,x,fuel,&effects,&v))return 0;*out=v;return 1;}
/* Emission uses fixed memory operand slots, never an ABI-misaligned push stack.
   Each actual call forwards one shared fuel/status pair. Runtime checks are a
   stricter teaching contract, not a claim that ISO C diagnoses pointer UB. */
typedef struct{char*b;size_t cap,n;int ok;} Writer;
static void put(Writer*w,const char*fmt,...){int n;va_list ap;if(!w->ok)return;va_start(ap,fmt);n=vsnprintf(w->b+w->n,w->cap-w->n,fmt,ap);va_end(ap);if(n<0||(size_t)n>=w->cap-w->n){w->ok=0;return;}w->n+=(size_t)n;}
static unsigned so(unsigned i){return 160+4*i;}
int ce_emit_thumb1(const CEProgram*,char*,size_t);
int ce_emit(const CEProgram*p,unsigned target,char*out,size_t cap){unsigned h[CE_CODE],f,pc,i,sp,z;char stage[131072];Writer w={stage,sizeof stage,0,1};if(target==2)return ce_emit_thumb1(p,out,cap);if(!out||target>1||!ce_verify(p,h))return 0;
 put(&w,target?".syntax unified\n.thumb\n.text\n":".text\n");
 for(f=0;f<p->functions;++f){const CEFunc*fn=&p->fn[f];put(&w,target?".thumb_func\n":"");if(f+1==p->functions)put(&w,".globl cm_ext\ncm_ext:\n");put(&w,"ce_f%u:\n",f);
 if(!target){put(&w,"pushq %%rbp\nmovq %%rsp,%%rbp\nsubq $512,%%rsp\nmovq %%rsi,-8(%%rbp)\nmovq %%rdx,-16(%%rbp)\nmovl %%edi,-32(%%rbp)\n");for(i=1;i<CE_LOCALS;++i)put(&w,"movl $0,-%u(%%rbp)\n",32+4*i);for(i=0;i<CE_ARRAY;++i)put(&w,"movl $0,-%u(%%rbp)\n",96+4*i);}
 else{put(&w,"push {r4-r7,lr}\nsub sp,#508\nmov r7,sp\nstr r1,[r7,#8]\nstr r2,[r7,#16]\nstr r0,[r7,#32]\nstr r3,[r7,#24]\nmovs r3,#0\n");for(i=1;i<CE_LOCALS;++i)put(&w,"str r3,[r7,#%u]\n",32+4*i);for(i=0;i<CE_ARRAY;++i)put(&w,"str r3,[r7,#%u]\n",96+4*i);}
 if(!target){if(f+1==p->functions)put(&w,"movl $0,-136(%%rbp)\nleaq -136(%%rbp),%%rcx\n");put(&w,"movq %%rcx,-24(%%rbp)\n");}else{if(f+1==p->functions)put(&w,"str r3,[r7,#136]\nadd r3,r7,#136\nstr r3,[r7,#24]\n");}
 for(pc=fn->begin;pc<fn->end;++pc){CEIns a=p->code[pc];sp=h[pc];if(sp==UINT_MAX)continue;put(&w,".L%u:\n",pc);
 if(!target)put(&w,"movq -8(%%rbp),%%rsi\ncmpl $0,(%%rsi)\nje .Lfail%u\nsubl $1,(%%rsi)\n",f);
 else put(&w,"ldr r1,[r7,#8]\nldr r3,[r1]\ncmp r3,#0\nbeq.w .Lfail%u\nsubs r3,#1\nstr r3,[r1]\n",f);
 z=sp?so(sp-1):0;
 if(!target){switch(a.op){case CE_CONST:put(&w,"movl $%u,-%u(%%rbp)\n",a.arg,so(sp));break;case CE_LOAD:put(&w,"movl -%u(%%rbp),%%eax\nmovl %%eax,-%u(%%rbp)\n",32+4*a.arg,so(sp));break;case CE_STORE:put(&w,"movl -%u(%%rbp),%%eax\nmovl %%eax,-%u(%%rbp)\n",z,32+4*a.arg);break;
 case CE_ADD:case CE_SUB:case CE_MUL:case CE_LT:case CE_EQ:put(&w,"movl -%u(%%rbp),%%eax\nmovl -%u(%%rbp),%%ecx\n",so(sp-2),z);if(a.op==CE_ADD)put(&w,"addl %%ecx,%%eax\n");else if(a.op==CE_SUB)put(&w,"subl %%ecx,%%eax\n");else if(a.op==CE_MUL)put(&w,"imull %%ecx,%%eax\n");else put(&w,"cmpl %%ecx,%%eax\n%s %%al\nmovzbl %%al,%%eax\n",a.op==CE_LT?"setb":"sete");put(&w,"movl %%eax,-%u(%%rbp)\n",so(sp-2));break;
 case CE_TO_I32:put(&w,"cmpl $2147483647,-%u(%%rbp)\nja .Lfail%u\n",z,f);break;case CE_TO_U32:break;
 case CE_SADD:case CE_SSUB:case CE_SMUL:case CE_SLT:case CE_SEQ:put(&w,"movl -%u(%%rbp),%%eax\nmovl -%u(%%rbp),%%ecx\n",so(sp-2),z);if(a.op==CE_SLT||a.op==CE_SEQ)put(&w,"cmpl %%ecx,%%eax\n%s %%al\nmovzbl %%al,%%eax\n",a.op==CE_SLT?"setl":"sete");else put(&w,"%s %%ecx,%%eax\njo .Lfail%u\n",a.op==CE_SADD?"addl":a.op==CE_SSUB?"subl":"imull",f);put(&w,"movl %%eax,-%u(%%rbp)\n",so(sp-2));break;
 case CE_ADDR:put(&w,"cmpl $8,-%u(%%rbp)\nja .Lfail%u\n",z,f);break;
 case CE_PADD:put(&w,"movl -%u(%%rbp),%%eax\nmovl $8,%%ecx\nsubl %%eax,%%ecx\ncmpl %%ecx,-%u(%%rbp)\nja .Lfail%u\naddl -%u(%%rbp),%%eax\nmovl %%eax,-%u(%%rbp)\n",so(sp-2),z,f,z,so(sp-2));break;
 case CE_READ:put(&w,"movl -%u(%%rbp),%%eax\ncmpl $8,%%eax\njae .Lfail%u\nnegq %%rax\nmovl -96(%%rbp,%%rax,4),%%eax\nmovl %%eax,-%u(%%rbp)\n",z,f,z);break;
 case CE_WRITE:put(&w,"movl -%u(%%rbp),%%eax\ncmpl $8,%%eax\njae .Lfail%u\nnegq %%rax\nmovl -%u(%%rbp),%%ecx\nmovl %%ecx,-96(%%rbp,%%rax,4)\n",so(sp-2),f,z);break;
 case CE_JZ:put(&w,"cmpl $0,-%u(%%rbp)\nje .L%u\n",z,a.arg);break;case CE_JMP:put(&w,"jmp .L%u\n",a.arg);break;
 case CE_TICK:put(&w,"movq -24(%%rbp),%%rcx\nmovl (%%rcx),%%eax\naddl %%eax,-%u(%%rbp)\naddl $1,(%%rcx)\n",z);break;case CE_CALL:put(&w,"movl -%u(%%rbp),%%edi\nmovq -16(%%rbp),%%rdx\nmovq -24(%%rbp),%%rcx\ncall ce_f%u\nmovq -16(%%rbp),%%rdx\ncmpl $0,(%%rdx)\njne .Lend%u\nmovl %%eax,-%u(%%rbp)\n",z,a.arg,f,z);break;
 case CE_RET:put(&w,"movl -%u(%%rbp),%%eax\njmp .Lend%u\n",z,f);break;}}
 else{switch(a.op){case CE_CONST:put(&w,"movw r0,#%u\nmovt r0,#%u\nstr r0,[r7,#%u]\n",a.arg&65535,a.arg>>16,so(sp));break;case CE_LOAD:put(&w,"ldr r0,[r7,#%u]\nstr r0,[r7,#%u]\n",32+4*a.arg,so(sp));break;case CE_STORE:put(&w,"ldr r0,[r7,#%u]\nstr r0,[r7,#%u]\n",z,32+4*a.arg);break;
 case CE_ADD:case CE_SUB:case CE_MUL:case CE_LT:case CE_EQ:put(&w,"ldr r0,[r7,#%u]\nldr r1,[r7,#%u]\n",so(sp-2),z);if(a.op==CE_ADD)put(&w,"adds r0,r0,r1\n");else if(a.op==CE_SUB)put(&w,"subs r0,r0,r1\n");else if(a.op==CE_MUL)put(&w,"mul r0,r0,r1\n");else put(&w,"cmp r0,r1\nmov.w r0,#0\nit %s\nmov%s r0,#1\n",a.op==CE_LT?"lo":"eq",a.op==CE_LT?"lo":"eq");put(&w,"str r0,[r7,#%u]\n",so(sp-2));break;
 case CE_TO_I32:put(&w,"ldr r0,[r7,#%u]\ncmp r0,#0\nbmi.w .Lfail%u\n",z,f);break;case CE_TO_U32:break;
 case CE_SADD:case CE_SSUB:case CE_SMUL:case CE_SLT:case CE_SEQ:put(&w,"ldr r0,[r7,#%u]\nldr r1,[r7,#%u]\n",so(sp-2),z);if(a.op==CE_SLT||a.op==CE_SEQ)put(&w,"cmp r0,r1\nmov.w r0,#0\nit %s\nmov%s r0,#1\n",a.op==CE_SLT?"lt":"eq",a.op==CE_SLT?"lt":"eq");else if(a.op==CE_SMUL)put(&w,"smull r2,r3,r0,r1\nasr r1,r2,#31\ncmp r1,r3\nbne.w .Lfail%u\nmov r0,r2\n",f);else put(&w,"%s r0,r0,r1\nbvs.w .Lfail%u\n",a.op==CE_SADD?"adds":"subs",f);put(&w,"str r0,[r7,#%u]\n",so(sp-2));break;
 case CE_ADDR:put(&w,"ldr r0,[r7,#%u]\ncmp r0,#8\nbhi.w .Lfail%u\n",z,f);break;
 case CE_PADD:put(&w,"ldr r0,[r7,#%u]\nldr r1,[r7,#%u]\nrsb r2,r0,#8\ncmp r1,r2\nbhi.w .Lfail%u\nadds r0,r0,r1\nstr r0,[r7,#%u]\n",so(sp-2),z,f,so(sp-2));break;
 case CE_READ:put(&w,"ldr r0,[r7,#%u]\ncmp r0,#8\nbhs.w .Lfail%u\nadd r1,r7,#96\nldr r0,[r1,r0,lsl #2]\nstr r0,[r7,#%u]\n",z,f,z);break;
 case CE_WRITE:put(&w,"ldr r0,[r7,#%u]\ncmp r0,#8\nbhs.w .Lfail%u\nldr r1,[r7,#%u]\nadd r2,r7,#96\nstr r1,[r2,r0,lsl #2]\n",so(sp-2),f,z);break;
 case CE_JZ:put(&w,"ldr r0,[r7,#%u]\ncmp r0,#0\nbeq.w .L%u\n",z,a.arg);break;case CE_JMP:put(&w,"b.w .L%u\n",a.arg);break;
 case CE_TICK:put(&w,"ldr r1,[r7,#24]\nldr r2,[r1]\nldr r0,[r7,#%u]\nadds r0,r0,r2\nstr r0,[r7,#%u]\nadds r2,#1\nstr r2,[r1]\n",z,z);break;case CE_CALL:put(&w,"ldr r0,[r7,#%u]\nldr r1,[r7,#8]\nldr r2,[r7,#16]\nldr r3,[r7,#24]\nbl ce_f%u\nldr r2,[r7,#16]\nldr r3,[r2]\ncmp r3,#0\nbne.w .Lend%u\nstr r0,[r7,#%u]\n",z,a.arg,f,z);break;
 case CE_RET:put(&w,"ldr r0,[r7,#%u]\nb.w .Lend%u\n",z,f);break;}}
 }
 if(!target)put(&w,".Lfail%u:\nmovq -16(%%rbp),%%rdx\nmovl $1,(%%rdx)\nxorl %%eax,%%eax\n.Lend%u:\nleave\nret\n",f,f);
 else put(&w,".Lfail%u:\nldr r2,[r7,#16]\nmovs r3,#1\nstr r3,[r2]\nmovs r0,#0\n.Lend%u:\nadd sp,#508\npop {r4-r7,pc}\n",f,f);
 }if(!target)put(&w,".section .note.GNU-stack,\"\",@progbits\n");if(!w.ok||w.n+1>cap)return 0;memcpy(out,stage,w.n+1);return 1;}
