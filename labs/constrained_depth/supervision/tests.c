#include "supervision.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){printf("CS_CHECK:%u:%s\n",__LINE__,#x);return 1;}}while(0)
static unsigned inject_bits,inject_epoch;
void cs_bitmap_hook(struct cs_bitmap*w){if(inject_bits){inject_bits=0;(void)cs_bitmap_progress(w,0);}}
void cs_snapshot_hook(struct cs_world*w){if(inject_epoch){inject_epoch=0;(void)cs_progress(w,0);}}
int main(void){
 struct cs_world w;CHECK(cs_init(&w,2,7)==CS_OK);CHECK(cs_window(&w)==CS_WAIT);
 CHECK(cs_progress(&w,0)==CS_OK&&cs_progress(&w,1)==CS_OK&&cs_window(&w)==CS_OK);
 CHECK(cs_progress(&w,0)==CS_OK&&cs_window(&w)==CS_WAIT&&w.last[0]==1&&w.last[1]==1);
 CHECK(cs_progress(&w,1)==CS_OK&&cs_window(&w)==CS_OK);
 CHECK(cs_progress(&w,0)==CS_OK&&cs_progress(&w,1)==CS_OK&&cs_window(&w)==CS_OK);
 CHECK(cs_window(&w)==CS_WAIT); /* Timer scheduling alone advances no epoch. */
 CHECK(cs_progress(&w,0)==CS_OK&&cs_progress(&w,1)==CS_OK);inject_epoch=1;
 CHECK(cs_window(&w)==CS_BUSY&&w.last[0]==3&&w.last[1]==3);CHECK(cs_window(&w)==CS_OK);
 atomic_store(&w.sequence,UINT_MAX-3);CHECK(cs_progress(&w,0)==CS_OK&&atomic_load(&w.sequence)==UINT_MAX-1);CHECK(cs_progress(&w,1)==CS_EXHAUSTED);
 CHECK(cs_init(&w,3,8)==CS_OK);CHECK(cs_progress(&w,0)==CS_OK&&cs_progress(&w,1)==CS_OK&&cs_window(&w)==CS_WAIT);CHECK(cs_progress(&w,2)==CS_OK&&cs_window(&w)==CS_OK);
 /* Defined false-health counterexample: announcing before commit. */
 CHECK(cs_init(&w,2,9)==CS_OK);unsigned committed=0;CHECK(cs_progress(&w,0)==CS_OK&&cs_progress(&w,1)==CS_OK&&cs_window(&w)==CS_OK&&committed==0);
 CHECK(cs_init(&w,2,10)==CS_OK);committed=1;CHECK(cs_progress(&w,0)==CS_OK&&cs_window(&w)==CS_WAIT&&committed==1);
 struct cs_bitmap b;CHECK(cs_bitmap_init(&b,2)==CS_OK);CHECK(cs_bitmap_progress(&b,0)==CS_OK&&cs_bitmap_window(&b)==CS_WAIT);CHECK(cs_bitmap_progress(&b,1)==CS_OK&&cs_bitmap_window(&b)==CS_OK);
 CHECK(cs_bitmap_progress(&b,0)==CS_OK&&cs_bitmap_progress(&b,1)==CS_OK);inject_bits=1;CHECK(cs_bitmap_window(&b)==CS_OK&&atomic_load(&b.pending)==1);CHECK(cs_bitmap_progress(&b,1)==CS_OK&&cs_bitmap_window(&b)==CS_OK);
 struct cs_recovery r;CHECK(cs_recovery_init(&r)==CS_OK);
 for(unsigned i=1;i<=3;++i)CHECK(cs_failure(&r,(enum cs_reason)i)==CS_RETRY&&r.attempts==i);
 CHECK(cs_failure(&r,CS_BOOT_FAILED)==CS_SAFE&&r.terminal&&r.attempts==3);struct cs_recovery terminal=r;CHECK(cs_failure(&r,CS_HUNG)==CS_SAFE&&memcmp(&r,&terminal,sizeof r)==0);
 for(unsigned word=0;word<8;++word)for(unsigned bit=0;bit<32;++bit){CHECK(cs_recovery_init(&r)==CS_OK);uint32_t words[8];memcpy(words,&r,sizeof r);words[word]^=UINT32_C(1)<<bit;memcpy(&r,words,sizeof r);CHECK(cs_failure(&r,CS_HUNG)==CS_SAFE&&r.terminal&&r.reason==CS_CORRUPT);}
 struct cs_power p[3]={{3300,10000,2000},{3300,1000,8000},{3300,20000,500}};uint64_t energy=99;CHECK(cs_energy(p,2,&energy)==CS_OK&&energy==UINT64_C(92400000000));CHECK(cs_energy(p,3,&energy)==CS_OK&&energy==UINT64_C(125400000000));
 struct cs_power batched[3]={{3300,10000,4000},{3300,1000,16000},{3300,20000,500}};CHECK(cs_energy(batched,3,&energy)==CS_OK&&energy==UINT64_C(217800000000)&&energy/2==UINT64_C(108900000000));
 p[0].millivolts=UINT64_MAX;energy=99;CHECK(cs_energy(p,3,&energy)==CS_EXHAUSTED&&energy==99);CHECK(cs_energy(0,0,&energy)==CS_OK&&energy==0);
 puts("CS_PASS:epochs=1,0,1,1 tasks3=stall before-commit=false-health coherent=BUSY bitmap=retained retries=3 corruption=256 energy-fJ=92400000000,125400000000");return 0;
}
