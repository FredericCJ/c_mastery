#include "sketch.h"
#include <stdio.h>
#include <string.h>
#ifndef SK_TEST
#define SK_TEST 1
#endif
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"SK_CHECK:%d:%s\n",__LINE__,#x); return 1; } } while(0)
#if SK_TEST==1
int main(void) {
    SkCmConfig config={0}; SkCm a,b,all,old; unsigned i,r; uint64_t value;
    config.p=3; config.depth=3;
    for(r=0;r<3;r++) { config.bias[r]=r; for(i=0;i<3;i++) config.mask[r][i]=UINT32_C(1)<<(i+r); }
    CHECK(sk_cm_init(&a,&config)==SK_OK); CHECK(sk_cm_init(&b,&config)==SK_OK); CHECK(sk_cm_init(&all,&config)==SK_OK);
    for(i=0;i<64;i++) {
        CHECK(sk_cm_add(i%2?&a:&b,i,(uint64_t)i+1)==SK_OK);
        CHECK(sk_cm_add(&all,i,(uint64_t)i+1)==SK_OK);
        CHECK(sk_cm_query(&all,i,&value)==SK_OK && value>=(uint64_t)i+1);
    }
    CHECK(sk_cm_merge(&a,&b)==SK_OK); CHECK(a.total==all.total);
    for(r=0;r<3;r++) for(i=0;i<8;i++) CHECK(a.cell[r][i]==all.cell[r][i]);
    old=a; b.config.mask[2][2]^=1; CHECK(sk_cm_merge(&a,&b)==SK_INVALID && memcmp(&a,&old,sizeof a)==0);
    CHECK(sk_cm_init(&a,&config)==SK_OK); CHECK(sk_cm_add(&a,9,UINT64_MAX)==SK_OK);
    old=a; CHECK(sk_cm_add(&a,10,1)==SK_OVERFLOW && memcmp(&a,&old,sizeof a)==0);
    CHECK(sk_cm_add(&a,9,0)==SK_OK && memcmp(&a,&old,sizeof a)==0);
    CHECK(sk_cm_merge(&a,&a)==SK_OVERFLOW && memcmp(&a,&old,sizeof a)==0);
    CHECK(sk_cm_query(&a,9,&value)==SK_OK && value==UINT64_MAX);
    config.p=0; CHECK(sk_cm_init(&a,&config)==SK_INVALID && memcmp(&a,&old,sizeof a)==0);
    config.p=1; config.depth=1; config.mask[0][0]=0; config.bias[0]=0;
    CHECK(sk_cm_init(&a,&config)==SK_OK); CHECK(sk_cm_add(&a,1,7)==SK_OK);
    CHECK(sk_cm_query(&a,99,&value)==SK_OK && value==7); /* explicit adversarial collision */
    puts("count_min: partition merge, all-or-nothing overflow, no-underestimate and collision checked"); return 0;
}
#elif SK_TEST==2
int main(void) {
    SkHll fine,coarse,direct,part,old; unsigned p,i; double estimate=123;
    for(p=4;p<=8;p++) {
        CHECK(sk_hll_init(&fine,p,91)==SK_OK); CHECK(sk_hll_estimate(&fine,&estimate)==SK_OK && estimate==0);
        CHECK(sk_hll_init(&part,p,91)==SK_OK); CHECK(sk_hll_init(&coarse,4,91)==SK_OK); CHECK(sk_hll_init(&direct,4,91)==SK_OK);
        for(i=0;i<256;i++) {
            uint32_t h=(uint32_t)i*UINT32_C(0x01000000);
            CHECK(sk_hll_add(i%2?&part:&fine,h)==SK_OK); CHECK(sk_hll_add(&direct,h)==SK_OK);
        }
        CHECK(sk_hll_merge(&fine,&part)==SK_OK); CHECK(sk_hll_fold(&coarse,&fine)==SK_OK);
        for(i=0;i<16;i++) CHECK(coarse.reg[i]==direct.reg[i]);
        old=fine; CHECK(sk_hll_merge(&fine,&fine)==SK_OK && memcmp(&fine,&old,sizeof fine)==0);
        part.identity++; CHECK(sk_hll_merge(&fine,&part)==SK_INVALID && memcmp(&fine,&old,sizeof fine)==0);
    }
    CHECK(sk_hll_init(&fine,8,91)==SK_OK); CHECK(sk_hll_add(&fine,0)==SK_OK && fine.reg[0]==25);
    CHECK(sk_hll_init(&coarse,4,91)==SK_OK); CHECK(sk_hll_fold(&coarse,&fine)==SK_OK && coarse.reg[0]==29);
    CHECK(sk_hll_add(&coarse,UINT32_C(0xf8000000))==SK_OK && coarse.reg[15]==1);
    old=fine; CHECK(sk_hll_fold(&fine,&coarse)==SK_INVALID && memcmp(&fine,&old,sizeof fine)==0);
    CHECK(sk_hll_init(&fine,4,91)==SK_OK);
    for(i=0;i<16;i++) fine.reg[i]=28;
    estimate=123; CHECK(sk_hll_estimate(&fine,&estimate)==SK_OK && estimate>1e9);
    for(i=0;i<16;i++) fine.reg[i]=29;
    estimate=123; CHECK(sk_hll_estimate(&fine,&estimate)==SK_OVERFLOW && estimate==123);
    puts("hyperloglog: exact rank, partition max, precision folding, empty and finite-domain correction checked"); return 0;
}
#elif SK_TEST==3
int main(void) {
    SkMhConfig config={0}; SkMh a,b,old,u; SkFraction f; unsigned i,j,bands=99;
    config.universe=5; config.count=10;
    for(i=0;i<10;i++) for(j=0;j<5;j++) config.rank[i][j]=(uint8_t)((i+j)%5);
    CHECK(sk_mh_init(&a,&config)==SK_OK); CHECK(sk_mh_init(&b,&config)==SK_OK);
    CHECK(sk_mh_similarity(&a,&b,&f)==SK_OK && f.numerator==10 && f.denominator==10);
    CHECK(sk_mh_bands(&a,&b,2,&bands)==SK_OK && bands==0);
    CHECK(sk_mh_add(&a,1)==SK_OK); CHECK(sk_mh_add(&b,2)==SK_OK);
    CHECK(sk_mh_similarity(&a,&b,&f)==SK_OK && f.numerator==0);
    CHECK(sk_mh_bands(&a,&b,2,&bands)==SK_OK && bands==0);
    CHECK(sk_mh_add(&a,2)==SK_OK); CHECK(sk_mh_add(&b,1)==SK_OK);
    CHECK(sk_mh_similarity(&a,&b,&f)==SK_OK && f.numerator==10);
    CHECK(sk_mh_bands(&a,&b,2,&bands)==SK_OK && bands==5);
    old=a; CHECK(sk_mh_add(&a,5)==SK_INVALID && memcmp(&a,&old,sizeof a)==0);
    bands=99; CHECK(sk_mh_bands(&a,&b,3,&bands)==SK_INVALID && bands==99);
    CHECK(sk_mh_merge(&a,&b)==SK_OK && memcmp(&a,&old,sizeof a)==0);
    CHECK(sk_mh_init(&u,&config)==SK_OK); CHECK(sk_mh_add(&u,4)==SK_OK); CHECK(sk_mh_merge(&a,&u)==SK_OK);
    for(i=0;i<10;i++) CHECK(a.minimum[i]==(old.minimum[i]<u.minimum[i]?old.minimum[i]:u.minimum[i]));
    old=a; b.config.rank[0][0]=1;b.config.rank[0][1]=0;
    CHECK(sk_mh_merge(&a,&b)==SK_INVALID && memcmp(&a,&old,sizeof a)==0);
    config.rank[0][0]=config.rank[0][1]; CHECK(sk_mh_init(&a,&config)==SK_INVALID && memcmp(&a,&old,sizeof a)==0);
    puts("minhash: true permutations, exact set union, empty policy and all-row band equality checked"); return 0;
}
#endif
