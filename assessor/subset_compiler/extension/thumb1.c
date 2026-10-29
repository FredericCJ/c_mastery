#include "extension.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
struct cc_tw {char *p;size_t n,cap;int ok;};
static void put(struct cc_tw*w,const char*f,...){if(!w->ok)return;va_list a;va_start(a,f);int n=vsnprintf(w->p+w->n,w->cap-w->n,f,a);va_end(a);if(n<0||(size_t)n>=w->cap-w->n)w->ok=0;else w->n+=(size_t)n;}
static unsigned so(unsigned n){return 160+4*n;}
/* Each long transfer has a nearby literal island, skipped on fallthrough;
 * explicit low bit makes the target Thumb state. No Thumb-2 branch assumed. */
static void jump(struct cc_tw*w,const char*k,unsigned n){put(w,"ldr r3,1f\nbx r3\n.align 2\n1: .word .L%s%u+1\n",k,n);}
static void cond(struct cc_tw*w,const char*op,const char*k,unsigned n){put(w,"b%s 2f\n",op);jump(w,k,n);put(w,"2:\n");}
int ce_emit_thumb1(const struct ce_program*p,char*out,size_t cap){char b[131072];struct cc_tw w={b,0,sizeof b,1};unsigned h[CE_CODE];if(!out||!ce_verify(p,h))return 0;put(&w,".syntax unified\n.thumb\n.cpu cortex-m0\n.text\n");
 for(unsigned f=0;f<p->functions;f++){const struct ce_func*fn=&p->fn[f];put(&w,".thumb_func\n");if(f+1==p->functions)put(&w,".global cm_ext\ncm_ext:\n");put(&w,"ce_f%u:\npush {r4-r7,lr}\nsub sp,#508\nstr r1,[sp,#8]\nstr r2,[sp,#16]\nstr r0,[sp,#32]\nstr r3,[sp,#24]\nmovs r3,#0\n",f);for(unsigned i=1;i<CE_LOCALS;i++)put(&w,"str r3,[sp,#%u]\n",32+4*i);for(unsigned i=0;i<CE_ARRAY;i++)put(&w,"str r3,[sp,#%u]\n",96+4*i);
  if(f+1==p->functions)put(&w,"str r3,[sp,#136]\nadd r3,sp,#136\nstr r3,[sp,#24]\n");
  for(unsigned pc=fn->begin;pc<fn->end;pc++){if(h[pc]==UINT_MAX)continue;struct ce_ins a=p->code[pc];unsigned sp=h[pc],z=sp?so(sp-1):0;put(&w,".L%u:\nldr r1,[sp,#8]\nldr r2,[r1]\ncmp r2,#0\n",pc);cond(&w,"ne","fail",f);put(&w,"subs r2,#1\nstr r2,[r1]\n");
   switch(a.op){case CE_CONST:put(&w,"movs r0,#%u\n",a.arg>>24);for(int shift=16;shift>=0;shift-=8)put(&w,"lsls r0,#8\nadds r0,#%u\n",(a.arg>>(unsigned)shift)&255);put(&w,"str r0,[sp,#%u]\n",so(sp));break;
   case CE_LOAD:put(&w,"ldr r0,[sp,#%u]\nstr r0,[sp,#%u]\n",32+4*a.arg,so(sp));break;
   case CE_STORE:put(&w,"ldr r0,[sp,#%u]\nstr r0,[sp,#%u]\n",z,32+4*a.arg);break;
   case CE_ADD:case CE_SUB:case CE_MUL:case CE_LT:case CE_EQ:put(&w,"ldr r0,[sp,#%u]\nldr r1,[sp,#%u]\n",so(sp-2),z);if(a.op==CE_ADD)put(&w,"adds r0,r0,r1\n");else if(a.op==CE_SUB)put(&w,"subs r0,r0,r1\n");else if(a.op==CE_MUL)put(&w,"muls r0,r1,r0\n");else put(&w,"cmp r0,r1\nb%s 3f\nmovs r0,#0\nb 4f\n3: movs r0,#1\n4:\n",a.op==CE_LT?"lo":"eq");put(&w,"str r0,[sp,#%u]\n",so(sp-2));break;
   case CE_TO_I32:put(&w,"ldr r0,[sp,#%u]\ncmp r0,#0\n",z);cond(&w,"pl","fail",f);break;case CE_TO_U32:break;
   case CE_SADD:case CE_SSUB:case CE_SMUL:case CE_SLT:case CE_SEQ:put(&w,"ldr r0,[sp,#%u]\nldr r1,[sp,#%u]\n",so(sp-2),z);if(a.op==CE_SLT||a.op==CE_SEQ)put(&w,"cmp r0,r1\nb%s 3f\nmovs r0,#0\nb 4f\n3: movs r0,#1\n4:\n",a.op==CE_SLT?"lt":"eq");else if(a.op==CE_SMUL){put(&w,"bl ce_smul\ncmp r1,#0\n");cond(&w,"eq","fail",f);}else{put(&w,"%s r0,r0,r1\n",a.op==CE_SADD?"adds":"subs");cond(&w,"vc","fail",f);}put(&w,"str r0,[sp,#%u]\n",so(sp-2));break;
   case CE_ADDR:put(&w,"ldr r0,[sp,#%u]\ncmp r0,#8\n",z);cond(&w,"ls","fail",f);break;
   case CE_PADD:put(&w,"ldr r0,[sp,#%u]\nldr r1,[sp,#%u]\nmovs r2,#8\nsubs r2,r2,r0\ncmp r1,r2\n",so(sp-2),z);cond(&w,"ls","fail",f);put(&w,"adds r0,r0,r1\nstr r0,[sp,#%u]\n",so(sp-2));break;
   case CE_READ:put(&w,"ldr r0,[sp,#%u]\ncmp r0,#8\n",z);cond(&w,"lo","fail",f);put(&w,"lsls r0,#2\nadd r2,sp,#96\nldr r0,[r2,r0]\nstr r0,[sp,#%u]\n",z);break;
   case CE_WRITE:put(&w,"ldr r0,[sp,#%u]\ncmp r0,#8\n",so(sp-2));cond(&w,"lo","fail",f);put(&w,"lsls r0,#2\nldr r1,[sp,#%u]\nadd r2,sp,#96\nstr r1,[r2,r0]\n",z);break;
   case CE_JZ:put(&w,"ldr r0,[sp,#%u]\ncmp r0,#0\n",z);cond(&w,"ne","",a.arg);break;
   case CE_JMP:jump(&w,"",a.arg);break;
   case CE_TICK:put(&w,"ldr r1,[sp,#24]\nldr r2,[r1]\nldr r0,[sp,#%u]\nadds r0,r0,r2\nstr r0,[sp,#%u]\nadds r2,#1\nstr r2,[r1]\n",z,z);break;case CE_CALL:put(&w,"ldr r0,[sp,#%u]\nldr r1,[sp,#8]\nldr r2,[sp,#16]\nldr r3,[sp,#24]\nbl ce_f%u\nldr r2,[sp,#16]\nldr r3,[r2]\ncmp r3,#0\n",z,a.arg);cond(&w,"eq","end",f);put(&w,"str r0,[sp,#%u]\n",z);break;
   case CE_RET:put(&w,"ldr r0,[sp,#%u]\n",z);jump(&w,"end",f);break;default:return 0;}
  }put(&w,".Lfail%u:\nldr r2,[sp,#16]\nmovs r3,#1\nstr r3,[r2]\nmovs r0,#0\n.Lend%u:\nadd sp,#508\npop {r4-r7,pc}\n",f,f);
 }
 /* Checked signed multiply using bounded unsigned magnitudes. No libgcc or
  * 64-bit instruction dependency; at most32 peasant-multiplication steps. */
 put(&w,".thumb_func\nce_smul:\npush {r4-r6,lr}\nmovs r4,r0\neors r4,r1\nlsrs r4,#31\ncmp r0,#0\nbpl 1f\nrsbs r0,r0,#0\n1: cmp r1,#0\nbpl 2f\nrsbs r1,r1,#0\n2: movs r3,#1\nlsls r3,#31\ncmp r4,#0\nbne 3f\nsubs r3,#1\n3: movs r2,#0\n4: cmp r1,#0\nbeq 8f\nmovs r5,#1\ntst r1,r5\nbeq 5f\nsubs r6,r3,r2\ncmp r0,r6\nbhi 9f\nadds r2,r2,r0\n5: lsrs r1,#1\nbeq 8f\ncmp r0,r3\nbhi 9f\nsubs r6,r3,r0\ncmp r0,r6\nbhi 9f\nlsls r0,#1\nb 4b\n8: movs r0,r2\ncmp r4,#0\nbeq 6f\nrsbs r0,r0,#0\n6: movs r1,#0\npop {r4-r6,pc}\n9: movs r1,#1\nmovs r0,#0\npop {r4-r6,pc}\n");
 if(!w.ok||w.n+1>cap)return 0;memcpy(out,b,w.n+1);return 1;}
