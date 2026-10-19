#include "shunting.h"
#include <stdio.h>
#include <time.h>
int main(void) {
    for(unsigned n=1;n<=61;n+=10) {
        unsigned char text[64];PdTokens t;PdDiagnostic d;PdTree tree;SyCost c;SyState s;
        clock_t start,end;unsigned copies=0;
        for(unsigned i=0;i<n;++i)text[i]=(i%2)?'+':'1';
        if(pd_lex(text,n,&t,&d))return 1;
        start=clock();
        for(unsigned k=0;k<10000;++k)if(sy_parse(&t,1,64,64,&tree,&c,&d))return 2;
        end=clock();
        printf("tokens=%u mode=whole iterations=10000 pushes=%u reductions=%u peak_op=%u peak_value=%u state_bytes=%zu ticks=%ld\n",n,c.pushes,c.reductions,c.peak_operators,c.peak_values,sizeof(SyState),(long)(end-start));
        start=clock();
        for(unsigned k=0;k<10000;++k) {
            if(sy_begin(1,64,64,&s))return 3;
            for(size_t i=0;i<t.count;++i) {int rc=sy_push(&s,&t.tokens[i],&d);if(rc!=PD_OK && rc!=PD_PENDING)return 4;++copies;}
        }
        end=clock();
        printf("tokens=%u mode=stream iterations=10000 staged_pushes=%u state_bytes=%zu ticks=%ld\n",n,copies,sizeof(SyState),(long)(end-start));
    }
    return 0;
}
