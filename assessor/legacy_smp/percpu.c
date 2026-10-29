#include "percpu.h"
#ifndef SP_MUTANT
#define SP_MUTANT 0
#endif
static int power(unsigned n){return n&&!(n&(n-1))&&n<=4096;}
static int add(uint64_t a,uint64_t b,uint64_t *out){if(b>UINT64_MAX-a)return 0;*out=a+b;return 1;}
static int aligned(uint64_t v,unsigned a,uint64_t *out){uint64_t sum;if(!add(v,a-1,&sum))return 0;*out=sum&~(uint64_t)(a-1);return 1;}
int sp_validate(const struct sp_span *spans,size_t count){
 if(!spans||count>4*SP_PARTS)return SP_INVALID;
 for(size_t i=0;i<count;++i){if(spans[i].low>=spans[i].high||spans[i].cpu>=4||spans[i].kind>=SP_PARTS)return SP_INVALID;
  for(size_t j=0;j<i;++j){if(spans[i].cpu==spans[j].cpu&&spans[i].kind==spans[j].kind)return SP_INVALID;
   if(SP_MUTANT!=1&&spans[i].low<spans[j].high&&spans[j].low<spans[i].high)return SP_ALIAS;}}
 return SP_OK;
}
int sp_layout(uint64_t base,uint64_t available,unsigned cpus,uint64_t stack_bytes,unsigned alignment,struct sp_layout *out){
 if(!out||cpus<1||cpus>4||!power(alignment)||alignment<8||stack_bytes<512||stack_bytes%alignment)return SP_INVALID;
 uint64_t limit;if(!add(base,available,&limit))return SP_RANGE;struct sp_layout staged={0};staged.cpus=cpus;uint64_t at=base;
 for(unsigned c=0;c<cpus;++c)for(unsigned kind=0;kind<SP_PARTS;++kind){int stack=kind<4||kind>=SP_TASK1;uint64_t size=stack?stack_bytes:kind==SP_GDT?56:kind==SP_TSS?104:4096;unsigned a=stack?alignment:kind==SP_IDT?16:8;uint64_t begin,end;
  if(!aligned(at,a,&begin)||!add(begin,size,&end))return SP_RANGE;
  if(SP_MUTANT!=2&&end>limit)return SP_SMALL;
  staged.spans[c*SP_PARTS+kind]=(struct sp_span){begin,end,c,kind};at=end;}
 staged.end=at;*out=staged;return SP_OK;
}
int sp_budget(const uint64_t *frames,size_t count,uint64_t hardware,uint64_t saved,unsigned alignment,uint64_t extended,uint64_t *out){
 if(!out||(!frames&&count)||count>16||!power(alignment))return SP_INVALID;
 uint64_t result=0;
 if(!add(hardware,saved,&result)||!add(result,alignment-1,&result)||!add(result,extended,&result))return SP_RANGE;
 for(size_t i=0;i<count;++i){uint64_t rounded;if(!aligned(frames[i],alignment,&rounded)||!add(result,rounded,&result))return SP_RANGE;}
 if(SP_MUTANT==3)result=hardware+saved;
 *out=result;return SP_OK;
}
