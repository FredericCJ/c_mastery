#include "query.h"
#include <inttypes.h>
#include <stdio.h>
static int read_row(QiRow *row)
{
    unsigned a,h,n;int v;if(scanf("%u%u%d%u",&a,&h,&v,&n)!=4||a>1||h>1||n>8||v<INT16_MIN||v>INT16_MAX)return 0;
    *row=(QiRow){0};row->alive=(uint8_t)a;row->has_value=(uint8_t)h;row->value=(int16_t)v;row->length=(uint8_t)n;
    for(unsigned i=0;i<n;i++){unsigned t;if(scanf("%u",&t)!=1||t>7)return 0;row->token[i]=(uint8_t)t;}
    return 1;
}
int main(void)
{
    unsigned n;
    while(scanf("%u",&n)==1){
        QiRow rows[32],replacement;QiIndex index;int change,lazy,lo,hi;unsigned required,forbidden,qn,a,b,d;uint8_t query[8];
        if(n>32)return 2;
        for(unsigned i=0;i<n;i++)if(!read_row(&rows[i]))return 2;
        if(scanf("%d%d",&change,&lazy)!=2)return 2;
        if(change>=0&&!read_row(&replacement))return 2;
        if(scanf("%u%u%d%d%u",&required,&forbidden,&lo,&hi,&qn)!=5||qn>8||lo<INT16_MIN||hi>INT16_MAX)return 2;
        for(unsigned i=0;i<qn;i++){unsigned t;if(scanf("%u",&t)!=1||t>7)return 2;query[i]=(uint8_t)t;}
        if(scanf("%u%u%u",&a,&b,&d)!=3)return 2;
        if(qi_build(rows,n,&index)!=QI_OK)return 3;
        if(change>=0&&qi_replace(&index,(size_t)change,replacement,lazy)!=QI_OK)return 3;
        uint32_t terms,phrase,near,range,composed;QiCost cost;QiTruth truth,negated,conjunction;
        if(qi_terms(&index,required,forbidden,&terms)||qi_phrase(&index,query,qn,&phrase)||qi_near(&index,a,b,d,&near)||qi_range(&index,(int16_t)lo,(int16_t)hi,&range,&cost)||qi_search(&index,required,forbidden,(int16_t)lo,(int16_t)hi,query,qn,&composed,&cost)||qi_compare(&index,(int16_t)lo,&truth))return 3;
        QiTruth total={terms,index.live&~terms};
        if(qi_logic(&index,truth,total,2,&negated)||qi_logic(&index,truth,total,0,&conjunction))return 3;
        printf("%"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %u %u\n",terms,phrase,near,range,composed,truth.yes,truth.no,negated.yes,negated.no,conjunction.yes,conjunction.no,cost.groups,cost.rows);
    }
    return 0;
}
