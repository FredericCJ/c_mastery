/* Hosted text adapter; all sketch mechanisms remain in the explicit provider. */
#include "sketch.h"
#include <inttypes.h>
#include <stdio.h>
int main(void) {
    char command;
    while(scanf(" %c",&command)==1) {
        unsigned i,j,n;
        if(command=='C') {
            SkCmConfig c={0}; SkCm s,part; uint64_t value;
            if(scanf("%u%u%u",&c.p,&c.depth,&n)!=3 || c.p>5 || c.depth>4) return 2;
            for(i=0;i<c.depth;i++) { if(scanf("%u",&c.bias[i])!=1) return 2;
                for(j=0;j<c.p;j++) if(scanf("%" SCNu32,&c.mask[i][j])!=1) return 2; }
            if(sk_cm_init(&s,&c)||sk_cm_init(&part,&c)) return 3;
            for(i=0;i<n;i++) { uint32_t key; uint64_t amount;
                if(scanf("%" SCNu32 "%" SCNu64,&key,&amount)!=2 || sk_cm_add(i%2?&part:&s,key,amount)) return 3; }
            if(sk_cm_merge(&s,&part)) return 3;
            printf("C %" PRIu64,s.total);
            for(i=0;i<c.depth;i++) for(j=0;j<(1u<<c.p);j++) printf(" %" PRIu64,s.cell[i][j]);
            for(i=0;i<64;i++) { if(sk_cm_query(&s,i,&value)) return 3; printf(" %" PRIu64,value); }
            puts("");
        } else if(command=='H') {
            SkHll fine,coarse; unsigned p,low; double estimate=123; int result;
            if(scanf("%u%u%u",&p,&low,&n)!=3 || sk_hll_init(&fine,p,1) || sk_hll_init(&coarse,low,1)) return 2;
            for(i=0;i<n;i++) { uint32_t h; if(scanf("%" SCNu32,&h)!=1 || sk_hll_add(&fine,h)) return 3; }
            if(sk_hll_fold(&coarse,&fine)) return 3;
            printf("H"); for(i=0;i<(1u<<p);i++) printf(" %u",(unsigned)fine.reg[i]);
            for(i=0;i<(1u<<low);i++) printf(" %u",(unsigned)coarse.reg[i]);
            result=sk_hll_estimate(&fine,&estimate); printf(" %d %.17g\n",result,estimate);
        } else if(command=='E') {
            SkHll h; unsigned p,v; double estimate=123; int result;
            if(scanf("%u%u",&p,&v)!=2 || v>255 || sk_hll_init(&h,p,1)) return 2;
            for(i=0;i<(1u<<p);i++) h.reg[i]=(uint8_t)v;
            result=sk_hll_estimate(&h,&estimate); printf("E %d %.17g\n",result,estimate);
        } else if(command=='M') {
            SkMhConfig c={0}; unsigned rows;
            if(scanf("%u%u%u%u",&c.universe,&c.count,&n,&rows)!=4 || c.universe>16 || c.count>120) return 2;
            for(i=0;i<c.count;i++) for(j=0;j<c.universe;j++) { unsigned v;if(scanf("%u",&v)!=1||v>255) return 2;c.rank[i][j]=(uint8_t)v; }
            for(i=0;i<n;i++) {
                SkMh a,b; SkFraction f; unsigned ma,mb,bands;
                if(scanf("%u%u",&ma,&mb)!=2 || sk_mh_init(&a,&c) || sk_mh_init(&b,&c)) return 3;
                for(j=0;j<c.universe;j++) { if((ma>>j)&1u) { if(sk_mh_add(&a,j)) return 3; } if((mb>>j)&1u) { if(sk_mh_add(&b,j)) return 3; } }
                if(sk_mh_similarity(&a,&b,&f)||sk_mh_bands(&a,&b,rows,&bands)||sk_mh_merge(&a,&b)) return 3;
                printf("M %u %u %u",f.numerator,f.denominator,bands);
                for(j=0;j<c.count;j++) { printf(" %u",(unsigned)a.minimum[j]); }
                puts("");
            }
        } else return 2;
    }
    return 0;
}
