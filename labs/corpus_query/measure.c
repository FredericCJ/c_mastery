#include "query.h"
#include <stdio.h>
#include <time.h>
int main(void)
{
    QiRow rows[32];QiIndex index;uint32_t out;QiCost cost;
    printf("index_bytes=%zu row_bytes=%zu posting_bytes=%zu zone_bytes=%zu\n",sizeof index,sizeof(QiRow),sizeof(QiPosting),sizeof(QiZone));
    for(unsigned order=0;order<2;order++){
        for(unsigned i=0;i<32;i++)rows[i]=(QiRow){1,1,2,{0,1},(int16_t)(order?(i*13)%32:i)};
        if(qi_build(rows,32,&index))return 1;
        clock_t start=clock();unsigned groups=0,examined=0,checksum=0;
        for(unsigned k=0;k<10000;k++){
            int16_t low=(int16_t)(k%32);uint8_t phrase[]={0,1};
            if(qi_search(&index,1,0,low,low,phrase,2,&out,&cost))return 1;
            groups+=cost.groups;examined+=cost.rows;checksum^=out;
        }
        clock_t stop=clock();if(start==(clock_t)-1||stop==(clock_t)-1)return 2;
        printf("order=%s queries=10000 groups=%u rows=%u cpu_ticks=%.0f ticks_per_second=%ld checksum=%u\n",order?"permuted":"sorted",groups,examined,(double)(stop-start),(long)CLOCKS_PER_SEC,checksum);
    }
    return 0;
}
