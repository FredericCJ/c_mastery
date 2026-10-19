#include "walker.h"
#include <stdio.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"B06_WALK_FAIL line=%d\n",__LINE__);return 1;}}while(0)
static PWMemory m;
static uint64_t pte(unsigned ppn,unsigned flags){return ((uint64_t)ppn<<10)|flags;}
static int permitted(unsigned f,unsigned s,unsigned sum,unsigned mxr,unsigned op){
 if(!(f&1)||((f&4)&&!(f&2)))return 0;
 if(!(f&10))return 0;
 if(!s&&!(f&16))return 0;
 if(s&&(f&16)&&(op==2||!sum))return 0;
 if(op==0&&!(f&2)&&!(mxr&&(f&8)))return 0;
 if(op==1&&!(f&4))return 0;
 if(op==2&&!(f&8))return 0;
 return (f&64)&&(op!=1||(f&128));
}
int main(void){
 unsigned cases=0;uint64_t out=91;memset(&m,0,sizeof m);m.present=7;
 C(pw_translate(NULL,0,0,1,0,0,0,&out)==PW_INVALID&&out==91);
 C(pw_translate(&m,1,0,1,0,0,0,&out)==PW_INVALID&&out==91);
 C(pw_translate(&m,0,0,2,0,0,0,&out)==PW_INVALID&&out==91);
 for(unsigned level=0;level<3;level++)for(unsigned f=0;f<256;f++)for(unsigned s=0;s<2;s++)for(unsigned sum=0;sum<2;sum++)for(unsigned mxr=0;mxr<2;mxr++)for(unsigned op=0;op<3;op++){
  memset(m.pte,0,sizeof m.pte);m.pte[0][0]=pte(1,1);m.pte[1][0]=pte(2,1);m.pte[2-level][0]=pte(0x80000,f);
  out=91;int got=pw_translate(&m,0,0x321,s,sum,mxr,op,&out);int want=permitted(f,s,sum,mxr,op);
  C((got==PW_OK)==want);C(want?out==UINT64_C(0x80000321):out==91);cases++;
 }
 for(unsigned level=0;level<3;level++)for(unsigned i=0;i<1024;i++){
  uint64_t offset=((uint64_t)i*7919)&((UINT64_C(1)<<(12+9*level))-1);
  memset(m.pte,0,sizeof m.pte);m.pte[0][0]=pte(1,1);m.pte[1][0]=pte(2,1);m.pte[2-level][0]=pte(0x80000,0xcf);
  out=91;C(pw_translate(&m,0,offset,1,0,0,0,&out)==PW_OK);C(out==UINT64_C(0x80000000)+offset);cases++;
 }
 memset(m.pte,0,sizeof m.pte);m.pte[0][0]=pte(0x80000,0xcf);
 C(pw_translate(&m,0,UINT64_C(1)<<39,1,0,0,0,&out)==PW_PAGE_FAULT);
 m.pte[0][256]=pte(0x80000,0xcf);C(pw_translate(&m,0,UINT64_C(0xffffffc000000123),1,0,0,0,&out)==PW_OK&&out==UINT64_C(0x80000123));
 m.pte[0][0]=pte(0x80001,0xcf);C(pw_translate(&m,0,0,1,0,0,0,&out)==PW_PAGE_FAULT);
 for(unsigned bit=54;bit<64;bit++){m.pte[0][0]=pte(0x80000,0xcf)|(UINT64_C(1)<<bit);C(pw_translate(&m,0,0,1,0,0,0,&out)==PW_PAGE_FAULT);}
 m.pte[0][0]=pte(1,1);m.pte[1][0]=pte(2,1);m.pte[2][0]=pte(0,1);C(pw_translate(&m,0,0,1,0,0,0,&out)==PW_PAGE_FAULT);
 for(unsigned f=16;f<=128;f*=2)if(f!=32){m.pte[0][0]=pte(1,1|f);C(pw_translate(&m,0,0,1,0,0,0,&out)==PW_PAGE_FAULT);}
 m.pte[0][0]=pte(1,1);m.read_blocked=2;C(pw_translate(&m,0,0,1,0,0,0,&out)==PW_ACCESS_FAULT);m.read_blocked=0;m.present=1;C(pw_translate(&m,0,0,1,0,0,0,&out)==PW_ACCESS_FAULT);m.present=7;
 m.pte[0][0]=pte(8,1);C(pw_translate(&m,0,0,1,0,0,0,&out)==PW_ACCESS_FAULT);
 printf("B06_WALK_PASS cases=%u\n",cases);return 0;
}
