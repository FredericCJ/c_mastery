#include "ecc.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ECC_CHECK:%s:%d\n",#x,__LINE__);return 41;}}while(0)
static int full(unsigned n) {
    unsigned limit=n==8?16u:2048u;unsigned long cases=0;
    for(unsigned data=0;data<limit;++data) {
        uint16_t word=0;struct ec_result r={99,99,99};CHECK(ec_encode(n,(uint16_t)data,&word)==EC_OK);
        CHECK(ec_decode(n,word,&r)==EC_OK&&r.data==data&&r.word==word&&r.position==0);++cases;
        for(unsigned i=0;i<n;++i) {
            CHECK(ec_decode(n,(uint16_t)(word^(1u<<i)),&r)==EC_CORRECTED);
            CHECK(r.data==data&&r.word==word&&r.position==i+1);++cases;
            for(unsigned j=i+1;j<n;++j) {
                r.data=99;r.word=99;r.position=99;
                CHECK(ec_decode(n,(uint16_t)(word^(1u<<i)^(1u<<j)),&r)==EC_DETECTED);
                CHECK(r.data==99&&r.word==99&&r.position==99);++cases;
            }
        }
    }
    uint16_t sentinel=123;struct ec_result unchanged={99,99,99};
    CHECK(ec_encode(n,(uint16_t)limit,&sentinel)==EC_INVALID&&sentinel==123);
    CHECK(ec_encode(7,0,&sentinel)==EC_INVALID&&sentinel==123);
    CHECK(ec_encode(n,0,(uint16_t *)0)==EC_INVALID);
    CHECK(ec_decode(n,0,(struct ec_result *)0)==EC_INVALID);
    if(n==8){CHECK(ec_decode(8,256,&unchanged)==EC_INVALID);CHECK(unchanged.data==99&&unchanged.word==99&&unchanged.position==99);}
    printf("ECC_FULL %u %lu\n",n,cases);return 0;
}
static int erasures(void) {
    for(unsigned n=8;n<=16;n+=8) {
        unsigned limit=n==8?16u:2048u;
        for(unsigned data=0;data<limit;++data) {
            uint16_t word=0;struct ec_result r={99,99,99};CHECK(ec_encode(n,(uint16_t)data,&word)==EC_OK);
            unsigned i=data%n,j=(i+1)%n;
            CHECK(ec_erasures(n,(uint16_t)(word^(1u<<j)),(uint16_t)(1u<<i),1,&r)==EC_CORRECTED);
            CHECK(r.data==data&&r.word==word);
        }
        /* All up-to-three erasure locations on zero, using linearity argument
         * separately; this is not mislabeled exhaustive all16-bit inputs. */
        for(unsigned i=0;i<n;++i)for(unsigned j=i+1;j<n;++j)for(unsigned k=j+1;k<n;++k) {
            uint16_t m=(uint16_t)((1u<<i)|(1u<<j)|(1u<<k));struct ec_result r={99,99,99};
            CHECK(ec_erasures(n,m,m,0,&r)==EC_CORRECTED&&r.word==0&&r.data==0);
        }
    }
    struct ec_result r={99,99,99};CHECK(ec_erasures(8,0,7,1,&r)==EC_INVALID);
    CHECK(r.data==99&&r.word==99&&r.position==99);
    CHECK(ec_erasures(8,3,0,1,&r)==EC_DETECTED);
    CHECK(r.data==99&&r.word==99&&r.position==99);
    puts("ECC_ERASURES");return 0;
}
static int burst(void) {
    for(unsigned n=8;n<=16;n+=8) {
        uint16_t words[4],received[4];uint8_t bits[64];
        for(unsigned lane=0;lane<4;++lane)CHECK(ec_encode(n,(uint16_t)(lane*3),&words[lane])==EC_OK);
        for(unsigned length=1;length<=4;++length)for(unsigned start=0;start+length<=4*n;++start) {
            CHECK(ec_interleave(n,words,bits)==EC_OK);
            for(unsigned i=start;i<start+length;++i)bits[i]^=1;
            CHECK(ec_deinterleave(n,bits,received)==EC_OK);
            for(unsigned lane=0;lane<4;++lane) {
                struct ec_result r={99,99,99};enum ec_status s=ec_decode(n,received[lane],&r);
                CHECK((s==EC_OK||s==EC_CORRECTED)&&r.data==lane*3);
            }
        }
        for(unsigned i=0;i<64;++i)bits[i]=7;
        CHECK(ec_interleave(n,words,bits)==EC_OK);
        for(unsigned i=4*n;i<64;++i)CHECK(bits[i]==7);
        bits[3]=2;
        for(unsigned i=0;i<4;++i)received[i]=99;
        CHECK(ec_deinterleave(n,bits,received)==EC_INVALID);
        for(unsigned i=0;i<4;++i)CHECK(received[i]==99);
        for(unsigned phase=0;phase<4;++phase) {
            for(unsigned start=0;start+4<=4*n;++start) {
                CHECK(ec_interleave_phase(n,words,phase,bits)==EC_OK);
                for(unsigned i=start;i<start+4;++i)bits[i]^=1;
                CHECK(ec_deinterleave_phase(n,bits,phase,received)==EC_OK);
                for(unsigned lane=0;lane<4;++lane) {
                    struct ec_result r={99,99,99};CHECK(ec_decode(n,received[lane],&r)==EC_CORRECTED&&r.data==lane*3);
                }
            }
        }
        CHECK(ec_interleave_phase(n,words,1,bits)==EC_OK);
        CHECK(ec_deinterleave_phase(n,bits,0,received)==EC_OK);
        struct ec_result wrong={99,99,99};CHECK(ec_decode(n,received[0],&wrong)==EC_OK&&wrong.data!=0);
        CHECK(ec_deinterleave_phase(n,bits,4,received)==EC_INVALID);
    }
    /* Three errors can be miscorrected; four carefully chosen flips are a
     * different valid word. Both are local synthetic property violations. */
    struct ec_result r={0,0,0};CHECK(ec_decode(8,7,&r)==EC_CORRECTED&&r.data!=0);
    CHECK(ec_decode(8,135,&r)==EC_OK&&r.data!=0);
    uint16_t forged=0;CHECK(ec_encode(8,9,&forged)==EC_OK);
    CHECK(ec_decode(8,forged,&r)==EC_OK&&r.data==9);
    puts("ECC_BURST_LIMITS");return 0;
}
int main(int argc,char **argv) {
    if(argc!=2)return 2;
    if(strcmp(argv[1],"8")==0)return full(8);
    if(strcmp(argv[1],"16")==0)return full(16);
    if(strcmp(argv[1],"erasures")==0)return erasures();
    if(strcmp(argv[1],"burst")==0)return burst();
    if(strcmp(argv[1],"oracle")==0) {
        for(unsigned w=0;w<256;++w){struct ec_result r={65535,65535,99};enum ec_status s=ec_decode(8,(uint16_t)w,&r);printf("8 %u %u %u %u\n",w,(unsigned)s,r.data,r.word);}
        for(unsigned d=0;d<2048;++d){uint16_t w=0;if(ec_encode(16,(uint16_t)d,&w)!=EC_OK)return 41;printf("16 %u %u\n",d,w);}
        return 0;
    }
    return 2;
}
