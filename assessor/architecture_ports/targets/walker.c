#include "walker.h"
#ifndef PW_MUTANT
#define PW_MUTANT 0
#endif
int pw_translate(const PWMemory*m,uint64_t root,uint64_t va,unsigned supervisor,unsigned sum,unsigned mxr,unsigned op,uint64_t*out){
 if(!m||!out||supervisor>1||sum>1||mxr>1||op>2||(root&4095)||(root>>56)||m->present>255||m->read_blocked>255)return PW_INVALID;
 uint64_t high=va>>39,sign=(va>>38)&1;
 if(PW_MUTANT!=1&&high!=(sign?UINT64_C(0x1ffffff):0))return PW_PAGE_FAULT;
 uint64_t table=root;
 for(int level=2;level>=0;--level){
  uint64_t page=table>>12;
  if(page>=8||!(m->present&(1u<<(unsigned)page))||(m->read_blocked&(1u<<(unsigned)page)))return PW_ACCESS_FAULT;
  uint64_t p=m->pte[page][(va>>(12+9*(unsigned)level))&511];
  unsigned v=(unsigned)(p&1),r=(unsigned)((p>>1)&1),w=(unsigned)((p>>2)&1),x=(unsigned)((p>>3)&1),u=(unsigned)((p>>4)&1),a=(unsigned)((p>>6)&1),d=(unsigned)((p>>7)&1);
  if(!v||(!r&&w)||(p>>54))return PW_PAGE_FAULT;
  uint64_t ppn=(p>>10)&UINT64_C(0xfffffffffff);
  if(!(r||x)){
   if(u||a||d||level==0)return PW_PAGE_FAULT;
   table=ppn<<12;continue;
  }
  uint64_t low=(UINT64_C(1)<<(9*(unsigned)level))-1;
  if(PW_MUTANT!=2&&(ppn&low))return PW_PAGE_FAULT;
  if(!supervisor&&!u)return PW_PAGE_FAULT;
  if(supervisor&&u&&((op==2&&PW_MUTANT!=3)||!sum))return PW_PAGE_FAULT;
  if((op==0&&!(r||(mxr&&x)))||(op==1&&!w)||(op==2&&!x))return PW_PAGE_FAULT;
  if(!a||(op==1&&!d))return PW_PAGE_FAULT;
  *out=((ppn&~low)<<12)|(va&((UINT64_C(1)<<(12+9*(unsigned)level))-1));return PW_OK;
 }
 return PW_PAGE_FAULT;
}
