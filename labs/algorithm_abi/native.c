#define _GNU_SOURCE
#include "bound.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"AB_ASSERT:%s\n",#x);return 41;}}while(0)
static uint32_t oracle(const uint32_t *a,uint32_t n,uint32_t key,unsigned upper){uint32_t i=0;while(i<n&&(a[i]<key||(upper&&a[i]==key)))++i;return i;}
static int search(void){
 const uint32_t a[5]={0,7,7,0x80000000u,UINT32_MAX},keys[5]={0,1,7,8,UINT32_MAX};uint32_t out=99,validation=99;
 CHECK(ab_checked(NULL,0,0,1,0,&out,&validation)==0 && out==0 && validation==0);
 for(unsigned i=0;i<5;++i){struct ab_trace t;CHECK(ab_trace(a,5,keys[i],0,&t)==0 && t.count==3 && t.index==oracle(a,5,keys[i],0));CHECK(ab_checked(a,5,5,keys[i],0,&out,&validation)==0 && out==t.index && validation==4);
#ifdef AB_WITH_C_ORACLE
  uint32_t prior=99;algo_work work={0};CHECK(algo_lower(a,5,keys[i],&prior,&work)==ALGO_OK && prior==out && work.comparisons==t.count);
#endif
  for(unsigned s=0;s<t.count;++s){struct ab_step p=t.steps[s];CHECK(p.lo<=p.mid && p.mid<p.hi && p.hi<=5 && p.next_hi-p.next_lo<p.hi-p.lo);for(unsigned j=0;j<p.lo;++j)CHECK(a[j]<keys[i]);for(unsigned j=p.hi;j<5;++j)CHECK(a[j]>=keys[i]);printf("TRACE %u %u %u %u %u %u %u\n",keys[i],s,p.lo,p.hi,p.mid,p.next_lo,p.next_hi);}
 }
 struct ab_trace lower,upper;CHECK(ab_trace(a,5,7,0,&lower)==0 && ab_trace(a,5,7,1,&upper)==0 && lower.index==1 && upper.index==3);
 uint32_t bad[2]={2,1};out=99;validation=99;CHECK(ab_checked(bad,2,2,1,0,&out,&validation)==1 && out==99 && validation==99);
 CHECK(ab_checked(a,5,4,1,0,&out,&validation)==1);CHECK(ab_checked(a,1025,1025,1,0,&out,&validation)==1);
 uint32_t single[1]={0x80000000u};CHECK(algo_asm_lower(single,1,UINT32_MAX)==1);CHECK(algo_asm_lower(single,1,0)==0);
 unsigned cases=0;uint32_t repeated[1024];for(unsigned i=0;i<1024;++i)repeated[i]=i/4;
 for(unsigned n=0;n<=1024;++n)for(unsigned q=0;q<5;++q){uint32_t key=q==4?UINT32_MAX:q*85;CHECK(ab_checked(repeated,n,n,key,1,&out,&validation)==0 && out==oracle(repeated,n,key,0) && validation==0);++cases;}
 CHECK(ab_checked(a,5,5,7,1,&out,&validation)==0 && out==1 && validation==0);
 puts("OBS search fixture-partitions upper1-to3 validation4-plus-search3 trusted-validates0 immutable-precondition");printf("CASES %u\n",cases);return 0;
}
static int abi(void){const uint32_t a[5]={0,7,7,0x80000000u,UINT32_MAX};uint32_t out;
 CHECK(algo_abi_probe(NULL,0,9,&out)==1 && out==0);
 CHECK(ab_callback_abi_probe(NULL,0,9,&out)==1 && out==0);
 for(unsigned i=0;i<5;++i){CHECK(algo_abi_probe(a,5,a[i],&out)==1 && out==oracle(a,5,a[i],0));CHECK(ab_callback_abi_probe(a,5,a[i],&out)==1 && out==oracle(a,5,a[i],0));}
 puts("OBS abi preserved-registers nested-alignment baseline-kernel");return 0;
}
static int callback(void){const uint32_t a[5]={0,7,7,0x80000000u,UINT32_MAX};
 for(unsigned poison=0;poison<2;++poison)for(unsigned i=0;i<5;++i){struct ab_context context={0,0,poison?0xdeadbeefu:0x13579bdu};struct ab_trace trace;CHECK(ab_trace(a,5,a[i],0,&trace)==0);CHECK(ab_callback_lower(a,5,a[i],ab_poison,&context)==trace.index && context.calls==trace.count && context.misaligned==0);}
 struct ab_context zero={0};CHECK(ab_callback_lower(NULL,0,0,ab_poison,&zero)==0 && zero.calls==0);
 puts("OBS callback original-assembly nested-live-state scratch-poison2 alignment");return 0;
}
static int windows(void){const uint32_t a[5]={0,7,7,0x80000000u,UINT32_MAX};
 CHECK(ab_ms_lower(NULL,0,9)==0);for(unsigned i=0;i<5;++i)CHECK(ab_ms_lower(a,5,a[i])==oracle(a,5,a[i],0));
 puts("OBS windows MS-register-and-shadow-store contract native-ms_abi not-Windows-OS");return 0;
}
static int records(void){const algo_pair a[5]={{0,10},{7,20},{7,30},{0x80000000u,40},{UINT32_MAX,50}};size_t out=99;
 CHECK(ab_records_checked(NULL,0,0,1,&out)==0 && out==0);CHECK(ab_records_checked(a,5,5,7,&out)==0 && out==1);CHECK(ab_records_checked(a,5,5,8,&out)==0 && out==3);
 out=99;CHECK(ab_records_checked(a,SIZE_MAX,SIZE_MAX,1,&out)==1 && out==99);CHECK(ab_records_checked(a,5,4,1,&out)==1 && out==99);
 puts("OBS records stride8 size_t admission-before-scaling preserves-failure");return 0;
}
static int guard(void){long page=sysconf(_SC_PAGESIZE);CHECK(page>0);
 for(unsigned layout=0;layout<2;++layout){size_t length=(size_t)page*(layout+2);unsigned char *base=mmap(NULL,length,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);CHECK(base!=MAP_FAILED);unsigned char *boundary=base+(layout+1)*(size_t)page;CHECK(mprotect(boundary,(size_t)page,PROT_NONE)==0);uint32_t *last=(uint32_t *)(boundary-sizeof(uint32_t));*last=7;
  for(unsigned nonempty=0;nonempty<2;++nonempty){pid_t child=fork();CHECK(child>=0);if(child==0){struct rlimit limit={0,0};if(setrlimit(RLIMIT_CORE,&limit))_exit(43);uint32_t result=algo_asm_lower(nonempty?last:(const uint32_t *)boundary,nonempty,8);_exit(result==nonempty?0:42);}int status;CHECK(waitpid(child,&status,0)==child);
#ifdef AB_EXPECT_OVERREAD
  CHECK(WIFSIGNALED(status) && WTERMSIG(status)==SIGSEGV);
#else
  CHECK(WIFEXITED(status) && WEXITSTATUS(status)==0);
#endif
  }
  CHECK(munmap(base,length)==0);
 }
 puts("OBS guard isolated-original-kernel layouts2 bounded-signal-classification");return 0;
}
static int scale(void){const uint32_t words[3]={0,7,100};CHECK(algo_asm_lower(words,2,8)==2);puts("OBS scale logical-two-elements physical-three-word-safe-discriminator");return 0;}
static int signedness(void){const uint32_t single[]={0x80000000u};CHECK(algo_asm_lower(single,1,0)==0);puts("OBS signedness one-element high-bit versus zero discriminator");return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"search"))return search();if(!strcmp(argv[1],"abi"))return abi();if(!strcmp(argv[1],"callback"))return callback();if(!strcmp(argv[1],"windows"))return windows();if(!strcmp(argv[1],"records"))return records();if(!strcmp(argv[1],"guard"))return guard();if(!strcmp(argv[1],"scale"))return scale();if(!strcmp(argv[1],"signedness"))return signedness();return 2;}
