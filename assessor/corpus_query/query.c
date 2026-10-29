#include "query.h"
#ifndef QI_MUTANT
#define QI_MUTANT 0
#endif
static int row_valid(struct qi_row r)
{
    if(r.alive>1||r.has_value>1||r.length>8)return 0;
    for(unsigned i=0;i<r.length;i++)if(r.token[i]>=8)return 0;
    return 1;
}
static int valid(const struct qi_index *x){return x&&x->count<=32;}
static uint32_t bit(unsigned r){return UINT32_C(1)<<r;}
static void add_zone(struct qi_zone *z,int16_t value)
{
    if(!z->present){z->minimum=z->maximum=value;z->present=1;}
    else{if(value<z->minimum)z->minimum=value;if(value>z->maximum)z->maximum=value;}
}
static void build_terms(struct qi_index *x)
{
    x->live=0;
    for(unsigned t=0;t<8;t++){x->term[t]=0;x->posting_count[t]=0;}
    for(size_t r=0;r<x->count;r++)if(x->row[r].alive){
        x->live|=bit((unsigned)r);
        for(unsigned t=0;t<8;t++){
            unsigned positions=0;
            for(unsigned p=0;p<x->row[r].length;p++)if(x->row[r].token[p]==t)positions|=1u<<p;
            if(positions){
                x->term[t]|=bit((unsigned)r);
                x->postings[t][x->posting_count[t]++]=(struct qi_posting){(uint8_t)r,(uint8_t)positions};
            }
        }
    }
}
int qi_rebuild_zones(struct qi_index *x)
{
    if(!valid(x))return QI_INVALID;
    for(unsigned g=0;g<8;g++)x->zone[g]=(struct qi_zone){0,0,0,1};
    for(size_t r=0;r<x->count;r++)if(x->row[r].alive&&x->row[r].has_value)add_zone(&x->zone[r/4],x->row[r].value);
    return QI_OK;
}
int qi_build(const struct qi_row *rows,size_t n,struct qi_index *out)
{
    if(!out||(!rows&&n)||n>32)return QI_INVALID;
    for(size_t r=0;r<n;r++)if(!row_valid(rows[r]))return QI_INVALID;
    struct qi_index next={0};next.count=n;
    for(size_t r=0;r<n;r++)next.row[r]=rows[r];
    build_terms(&next);(void)qi_rebuild_zones(&next);*out=next;return QI_OK;
}
int qi_replace(struct qi_index *x,size_t row,struct qi_row value,int lazy_zone)
{
    if(!valid(x)||row>=x->count||!row_valid(value)||(lazy_zone!=0&&lazy_zone!=1))return QI_INVALID;
    struct qi_index next=*x;next.row[row]=value;build_terms(&next);
    if(QI_MUTANT==2)for(unsigned t=0;t<8;t++)next.term[t]|=x->term[t];
    if(lazy_zone){
        struct qi_zone *z=&next.zone[row/4];z->exact=0;
        if(QI_MUTANT!=5&&value.alive&&value.has_value)add_zone(z,value.value);
    }else (void)qi_rebuild_zones(&next);
    *x=next;return QI_OK;
}
int qi_terms(const struct qi_index *x,unsigned required,unsigned forbidden,uint32_t *out)
{
    if(!valid(x)||!out||required>255||forbidden>255)return QI_INVALID;
    uint32_t answer=QI_MUTANT==1?UINT32_MAX:x->live;
    for(unsigned t=0;t<8;t++){
        if(required&(1u<<t))answer&=x->term[t];
        if(forbidden&(1u<<t))answer&=~x->term[t];
    }
    *out=answer;return QI_OK;
}
int qi_compare(const struct qi_index *x,int16_t threshold,struct qi_truth *out)
{
    if(!valid(x)||!out)return QI_INVALID;
    struct qi_truth result={0,0};
    for(size_t r=0;r<x->count;r++)if(x->row[r].alive&&x->row[r].has_value){
        if(x->row[r].value>=threshold)result.yes|=bit((unsigned)r);
        else result.no|=bit((unsigned)r);
    }
    *out=result;return QI_OK;
}
static int truth_valid(const struct qi_index *x,struct qi_truth a)
{return !(a.yes&a.no)&&!((a.yes|a.no)&~x->live);}
int qi_logic(const struct qi_index *x,struct qi_truth a,struct qi_truth b,unsigned operation,struct qi_truth *out)
{
    if(!valid(x)||!out||operation>2||!truth_valid(x,a)||!truth_valid(x,b))return QI_INVALID;
    struct qi_truth result;
    if(operation==0)result=(struct qi_truth){a.yes&b.yes,a.no|b.no};
    else if(operation==1)result=(struct qi_truth){a.yes|b.yes,a.no&b.no};
    else result=(struct qi_truth){a.no,a.yes};
    *out=result;return QI_OK;
}
static unsigned posting_positions(const struct qi_index *x,unsigned term,unsigned row)
{
    for(unsigned i=0;i<x->posting_count[term];i++){
        struct qi_posting p=x->postings[term][i];if(p.row==row)return p.positions;if(p.row>row)break;
    }
    return 0;
}
int qi_phrase(const struct qi_index *x,const uint8_t *query,size_t n,uint32_t *out)
{
    if(!valid(x)||!out||(!query&&n)||n>8)return QI_INVALID;
    for(size_t i=0;i<n;i++)if(query[i]>=8)return QI_INVALID;
    if(!n){*out=x->live;return QI_OK;}
    unsigned candidates[32],count=0;
    for(unsigned i=0;i<x->posting_count[query[0]];i++)candidates[count++]=x->postings[query[0]][i].row;
    /* Merge sorted document IDs before looking at positions. */
    for(size_t q=1;q<n;q++){
        unsigned a=0,b=0,next=0;
        while(a<count&&b<x->posting_count[query[q]]){
            unsigned r=x->postings[query[q]][b].row;
            if(candidates[a]==r){candidates[next++]=r;a++;b++;}
            else if(candidates[a]<r)a++;else b++;
        }
        count=next;
    }
    uint32_t result=0;
    for(unsigned i=0;i<count;i++){
        unsigned starts=255;
        for(size_t q=0;q<n;q++)starts&=posting_positions(x,query[q],candidates[i])>>q;
        if(QI_MUTANT==3||starts)result|=bit(candidates[i]);
    }
    *out=result;return QI_OK;
}
int qi_near(const struct qi_index *x,unsigned a,unsigned b,unsigned distance,uint32_t *out)
{
    if(!valid(x)||!out||a>=8||b>=8||distance>7)return QI_INVALID;
    uint32_t result=0;
    for(unsigned i=0;i<x->posting_count[a];i++){
        struct qi_posting p=x->postings[a][i];unsigned other=posting_positions(x,b,p.row);
        for(unsigned j=0;j<8;j++)if(p.positions&(1u<<j))for(unsigned k=0;k<8;k++)if(other&(1u<<k)){
            unsigned delta=j>k?j-k:k-j;
            if((QI_MUTANT==4||delta>0)&&delta<=distance)result|=bit(p.row);
        }
    }
    *out=result;return QI_OK;
}
int qi_range(const struct qi_index *x,int16_t low,int16_t high,uint32_t *out,struct qi_cost *cost)
{
    if(!valid(x)||!out||!cost||low>high)return QI_INVALID;
    struct qi_cost work={0,0};uint32_t result=0;
    for(size_t g=0;g<(x->count+3)/4;g++){
        struct qi_zone z=x->zone[g];
        if(!z.present||z.maximum<low||z.minimum>high)continue;
        if(QI_MUTANT==6&&(z.maximum==low||z.minimum==high))continue;
        work.groups++;
        for(size_t r=g*4;r<g*4+4&&r<x->count;r++){
            work.rows++;struct qi_row row=x->row[r];
            if(row.alive&&row.has_value&&row.value>=low&&row.value<=high)result|=bit((unsigned)r);
        }
    }
    *out=result;*cost=work;return QI_OK;
}
int qi_search(const struct qi_index *x,unsigned required,unsigned forbidden,int16_t low,int16_t high,const uint8_t *phrase,size_t n,uint32_t *out,struct qi_cost *cost)
{
    if(!out||!cost)return QI_INVALID;
    uint32_t a,b,c;struct qi_cost work;int status=qi_terms(x,required,forbidden,&a);
    if(status)return status;
    status=qi_phrase(x,phrase,n,&b);if(status)return status;
    status=qi_range(x,low,high,&c,&work);if(status)return status;
    *out=a&b&c;*cost=work;return QI_OK;
}
