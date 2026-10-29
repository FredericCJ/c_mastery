#include "dfa_cases.h"
#include <stdio.h>
#include <time.h>
int main(void){unsigned char text[256];struct mm_dfa d=mm_example();struct mm_workspace w;struct mm_tokens out;struct mm_cost c;
    for(unsigned k=1;k<=81;k+=10){unsigned n=3*k;for(unsigned i=0;i<n;++i)text[i]=(unsigned char)"abc"[i%3];
        for(unsigned mode=0;mode<2;++mode){clock_t start=clock();
            for(unsigned rep=0;rep<1000;++rep)if(mm_scan(&d,text,n,mode,256,&w,&out,&c,NULL))return 1;
            printf("bytes=%u mode=%u tokens=%u transitions=%u rollback=%u memo_stops=%u marked=%u scratch_bytes=%zu ticks=%ld\n",n,mode,out.count,c.transitions,c.rollback_bytes,c.memo_stops,c.marked_pairs,sizeof w,(long)(clock()-start));
        }
    }return 0;
}
