#include "depth.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef FD_C23
#include <stdckdint.h>
#endif
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"FD_COMPARE_CHECK:%d:%s\n",__LINE__,#x);return 1; } } while(0)
static enum fd_status builtin_extent(size_t o,size_t c,size_t w,size_t limit,size_t *out) {
    size_t product,end;
    if(!out) return FD_INVALID;
    if(__builtin_mul_overflow(c,w,&product) || __builtin_add_overflow(o,product,&end) || end>limit) return FD_RANGE;
    *out=end;return FD_OK;
}
#ifdef FD_C23
static enum fd_status checked_extent(size_t o,size_t c,size_t w,size_t limit,size_t *out) {
    size_t product,end;
    if(!out) return FD_INVALID;
    if(ckd_mul(&product,c,w) || ckd_add(&end,o,product) || end>limit) return FD_RANGE;
    *out=end;return FD_OK;
}
#endif
void fd_plain_transform(unsigned char *,const unsigned char *,size_t,unsigned char);
int main(void) {
    unsigned long cases=0;
    for(size_t o=0;o<16;++o) for(size_t c=0;c<16;++c)
    for(size_t w=0;w<16;++w) for(size_t limit=0;limit<16;++limit) {
        size_t a=917,b=917,exact=o+c*w;
        enum fd_status want=exact<=limit ? FD_OK : FD_RANGE;
        CHECK(fd_extent(o,c,w,limit,&a)==want);
        CHECK(builtin_extent(o,c,w,limit,&b)==want && a==b);
#ifdef FD_C23
        size_t d=917;CHECK(checked_extent(o,c,w,limit,&d)==want && a==d);
#endif
        CHECK(a==(want==FD_OK ? exact : 917));++cases;
    }
    const size_t edge[][4]={{SIZE_MAX,1,1,SIZE_MAX},{0,SIZE_MAX,2,SIZE_MAX},
        {SIZE_MAX,SIZE_MAX,0,SIZE_MAX},{SIZE_MAX,0,0,SIZE_MAX-1},
        {SIZE_MAX-1,1,1,SIZE_MAX},{0,0,SIZE_MAX,0}};
    const enum fd_status want[]={FD_RANGE,FD_RANGE,FD_OK,FD_RANGE,FD_OK,FD_OK};
    const size_t result[]={917,917,SIZE_MAX,917,SIZE_MAX,0};
    for(unsigned i=0;i<6;++i) {
        size_t out=917;
        CHECK(builtin_extent(edge[i][0],edge[i][1],edge[i][2],edge[i][3],&out)==want[i] && out==result[i]);
#ifdef FD_C23
        out=917;CHECK(checked_extent(edge[i][0],edge[i][1],edge[i][2],edge[i][3],&out)==want[i] && out==result[i]);
#endif
        ++cases;
    }
    size_t written=917;
    CHECK(__builtin_add_overflow(SIZE_MAX,(size_t)1,&written) && written==0);
#ifdef FD_C23
    written=917;CHECK(ckd_add(&written,SIZE_MAX,(size_t)1) && written==0);
#endif
    static unsigned char source[65536],restricted[65536],unrestricted[65536];
    for(size_t i=0;i<sizeof source;++i) source[i]=(unsigned char)i;
    const size_t sizes[]={4,64,4096,65536};
    unsigned long checksum=0;
    for(unsigned n=0;n<4;++n) for(unsigned sample=0;sample<5;++sample) {
        size_t length=sizes[n];
        fd_transform(restricted,source,length,0);
        fd_plain_transform(unrestricted,source,length,0);
        clock_t begin=clock();CHECK(begin!=(clock_t)-1);
        for(unsigned rep=0;rep<128;++rep) {
            if(sample%2) { fd_plain_transform(unrestricted,source,length,(unsigned char)rep);checksum+=unrestricted[rep%length]; }
            else { fd_transform(restricted,source,length,(unsigned char)rep);checksum+=restricted[rep%length]; }
        }
        clock_t mid=clock();CHECK(mid!=(clock_t)-1);
        for(unsigned rep=0;rep<128;++rep) {
            if(sample%2) { fd_transform(restricted,source,length,(unsigned char)rep);checksum+=restricted[rep%length]; }
            else { fd_plain_transform(unrestricted,source,length,(unsigned char)rep);checksum+=unrestricted[rep%length]; }
        }
        clock_t end=clock();CHECK(end!=(clock_t)-1);
        CHECK(!memcmp(restricted,unrestricted,length));
        printf("FD_CPU_SAMPLE size=%zu repetitions=128 sample=%u restricted_ticks=%ld plain_ticks=%ld clocks_per_second=%ld\n",
               length,sample,(long)(sample%2 ? end-mid : mid-begin),
               (long)(sample%2 ? mid-begin : end-mid),(long)CLOCKS_PER_SEC);
    }
    printf("FD_COMPARE_OK:%lu checksum=%lu edition=%ld\n",cases,checksum,(long)__STDC_VERSION__);
    return 0;
}
