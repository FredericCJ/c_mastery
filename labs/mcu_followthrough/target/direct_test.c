#include "target.h"
#include <stdatomic.h>
static _Atomic uint32_t atomic_word;
#ifdef MF_SPILL
#define LOCAL_WORDS 67u
#else
#define LOCAL_WORDS 19u
#endif
static volatile uint32_t initialized[3]={0x13579bdfu,0x2468ace0u,0x10203040u};
static volatile uint32_t moved __attribute__((section(".data2")))=0x89abcdefu;
static volatile uint32_t zeros[31];
static volatile uint32_t retained __attribute__((section(".retained")));
static uint32_t stacks[3][768] __attribute__((aligned(8)));
static uint32_t *saved[3],*main_sp;
static unsigned active,done[3],calls[3],max_depth[3];
static struct mf_probe probes[3];
static volatile uint64_t wide_n=UINT64_C(0xfedcba9876543210),wide_d=UINT64_C(0x100000003);
static volatile uint32_t narrow_n=0xfedcba98u,narrow_d=17u;
void mf_direct_yield(void){mf_direct_switch(&saved[active],main_sp);}
void mf_direct_return(unsigned task){mf_require(task==active,30);done[task]=1;
 mf_direct_yield();mf_require(0,31);}
static void nested(unsigned id,unsigned depth){
 volatile uint32_t locals[LOCAL_WORDS];uintptr_t sp;
 __asm__ volatile("mov %0,sp":"=r"(sp));mf_require((sp&7u)==0,32);
 for(unsigned j=0;j<LOCAL_WORDS;j++)locals[j]=0x1000u+id*100u+depth*20u+j;
 if(depth>max_depth[id])max_depth[id]=depth;
 if(depth)nested(id,depth-1);
 else{struct mf_probe *p=&probes[id];mf_direct_probe(p);
  for(unsigned j=0;j<8;j++)mf_require(p->integer[j]==p->observed_integer[j],40+j);
#if MF_FP
  for(unsigned j=0;j<16;j++)mf_require(p->high_fp[j]==p->observed_fp[j],60+j);
  mf_require(p->fpscr==p->observed_fpscr,80);
  mf_require(p->rounded==0x3f800000u+(((p->fpscr>>22)&3u)==1u),82);
#endif
  calls[id]++;
 }
 for(unsigned j=0;j<LOCAL_WORDS;j++)mf_require(locals[j]==0x1000u+id*100u+depth*20u+j,81);
}
static void entry(unsigned id){for(unsigned i=0;i<7u+id;i++)nested(id,id+1);}
void mf_main(void){
 mf_require(initialized[0]==0x13579bdfu&&initialized[1]==0x2468ace0u&&initialized[2]==0x10203040u,1);
 mf_require(moved==0x89abcdefu,2);for(unsigned j=0;j<31;j++)mf_require(zeros[j]==0,3);
 mf_require(retained==0x52544e44u,4);
 mf_require(mf_udiv32(0xffffffffu,1)==0xffffffffu&&mf_udiv32(1,0)==0xffffffffu,5);
 mf_require(mf_udiv64(UINT64_MAX,UINT64_C(0x8000000000000000))==1,6);
 mf_require(wide_n/wide_d==UINT64_C(0xfedcba95)&&wide_n%wide_d==UINT64_C(0x79be0251),7);
 mf_require(narrow_n/narrow_d==0x0efdecdbu&&narrow_n%narrow_d==13u,8);
 uint32_t expected=0;mf_require(atomic_compare_exchange_strong(&atomic_word,&expected,9)&&atomic_load(&atomic_word)==9,9);
 mf_text("MF startup two RAM data regions; poisoned BSS cleared; retained preserved\n");
 mf_text("MF original 32/64 division boundary\n");
 for(unsigned i=0;i<3;i++){
  for(unsigned j=0;j<768;j++)stacks[i][j]=0xa5a5a5a5u;
  for(unsigned j=0;j<8;j++)probes[i].integer[j]=0x11110000u+i*0x100u+j;
  for(unsigned j=0;j<16;j++)probes[i].high_fp[j]=0x3f800000u+i*0x10000u+j*16u;
  probes[i].fpscr=(i&3u)<<22;
  unsigned words=10u+(MF_FP?16u:0u);saved[i]=&stacks[i][768-words];
  for(unsigned j=0;j<words;j++)saved[i][j]=0;
  saved[i][1]=i;saved[i][2]=(uint32_t)(uintptr_t)entry;
  saved[i][9]=(uint32_t)(uintptr_t)mf_direct_enter;
#ifdef MF_BAD_ALIGN
  saved[i]--;
#endif
 }
 for(unsigned round=0;round<12;round++)for(unsigned i=0;i<3;i++)if(!done[i]){
  active=i;mf_direct_switch(&main_sp,saved[i]);
 }
 for(unsigned i=0;i<3;i++){
  mf_require(done[i]&&calls[i]==7u+i&&max_depth[i]==i+1,90+i);
  mf_require(stacks[i][0]==0xa5a5a5a5u,94);
  unsigned first=0;while(first<768&&stacks[i][first]==0xa5a5a5a5u)first++;
  mf_text("MF direct task/high-water-words ");mf_number(i);mf_number(768-first);
 }
 mf_text(MF_FP?"PASS MF direct FP32-high+FPSCR\n":"PASS MF direct integer\n");mf_finish(0);
}
