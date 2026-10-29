#include "depth.h"
#include <stdio.h>
#include <inttypes.h>
int main(void) {
    char op;
    printf("SIZE_MAX %zu\n",SIZE_MAX);
    while (scanf(" %c",&op)==1) {
        if (op=='E') {
            size_t o,c,w,l,result=917;
            if (scanf("%zu %zu %zu %zu",&o,&c,&w,&l)!=4) return 2;
            enum fd_status s=fd_extent(o,c,w,l,&result);
            printf("E %d %zu\n",s,result);
        } else if (op=='D') {
            unsigned char bytes[8];
            for (unsigned i=0;i<8;i++) { unsigned x; if (scanf("%x",&x)!=1 || x>255) return 2; bytes[i]=(unsigned char)x; }
            struct fd_record r={3,999,42}; enum fd_status s=fd_decode(bytes,8,&r);
            printf("D %d %u %u %" PRId32 "\n",s,(unsigned)r.flags,(unsigned)r.count,r.value);
        } else return 2;
    }
    return 0;
}
