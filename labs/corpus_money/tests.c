#include "money.h"
#include <stdio.h>
#include <string.h>
#define CHECK(v) do{if(!(v)){fprintf(stderr,"CM_CHECK:%d\n",__LINE__);return 1;}}while(0)
static struct cm_money m(int64_t x){struct cm_money r={x,{'U','S','D'},2};return r;}
static int currency(void)
{
    struct cm_money a,b=m(77),sentinel=b;
    CHECK(cm_make(1,"USD",2,&a)==CM_OK&&a.minor==1);
    CHECK(cm_make(1,"usd",2,&b)==CM_INVALID&&!memcmp(&b,&sentinel,sizeof b));
    CHECK(cm_make(1,"USD",10,&b)==CM_INVALID);
    CHECK(cm_add(m(INT64_MAX),m(1),&b)==CM_OVERFLOW&&b.minor==77);
    CHECK(cm_add(m(INT64_MIN),m(-1),&b)==CM_OVERFLOW&&b.minor==77);
    CHECK(cm_add(m(INT64_MIN),m(0),&b)==CM_OK&&b.minor==INT64_MIN);
    a=m(0);a.currency[0]='E';CHECK(cm_add(a,m(1),&b)==CM_INVALID);
    a=m(0);a.scale=3;CHECK(cm_add(a,m(1),&b)==CM_INVALID);
    return 0;
}
static int rounding(void)
{
    struct cm_money b=m(77);
    CHECK(cm_ratio(m(5),1,2,CM_EVEN,&b)==CM_OK&&b.minor==2);
    CHECK(cm_ratio(m(7),1,2,CM_EVEN,&b)==CM_OK&&b.minor==4);
    CHECK(cm_ratio(m(-5),1,2,CM_FLOOR,&b)==CM_OK&&b.minor==-3);
    CHECK(cm_ratio(m(-5),1,2,CM_ZERO,&b)==CM_OK&&b.minor==-2);
    CHECK(cm_ratio(m(INT64_MIN),UINT32_MAX,UINT32_MAX,CM_EVEN,&b)==CM_OK&&b.minor==INT64_MIN);
    b=m(77);CHECK(cm_ratio(m(INT64_MAX),2,1,CM_ZERO,&b)==CM_OVERFLOW&&b.minor==77);
    CHECK(cm_ratio(m(0),1,0,CM_EVEN,&b)==CM_INVALID&&b.minor==77);
    CHECK(cm_ratio(m(INT64_MIN),0,1,CM_ZERO,&b)==CM_OK&&b.minor==0);
    return 0;
}
static int allocation(void)
{
    uint32_t w[]={1,1,1,0};struct cm_money b[4]={m(77),m(77),m(77),m(77)};
    CHECK(cm_allocate(m(2),w,4,b)==CM_OK&&b[0].minor==1&&b[1].minor==1&&b[2].minor==0&&b[3].minor==0);
    CHECK(cm_allocate(m(-2),w,4,b)==CM_OK&&b[0].minor==-1&&b[1].minor==-1&&b[2].minor==0);
    w[0]=UINT32_MAX;w[1]=w[2]=0;
    CHECK(cm_allocate(m(INT64_MIN),w,4,b)==CM_OK&&b[0].minor==INT64_MIN&&b[1].minor==0);
    struct cm_money old[4];for(size_t i=0;i<4;i++)old[i]=b[i];w[1]=1;
    CHECK(cm_allocate(m(4),w,4,b)==CM_OVERFLOW&&!memcmp(b,old,sizeof b));
    w[0]=w[1]=0;CHECK(cm_allocate(m(4),w,4,b)==CM_INVALID&&!memcmp(b,old,sizeof b));
    return 0;
}
static int settlement(void)
{
    struct cm_money a[]={m(10),m(0),m(0),m(0)},old[4];size_t dest[]={1,2,3};uint32_t w[]={1,1,1};
    CHECK(cm_settle(a,4,0,dest,w,3,m(2),SIZE_MAX)==CM_OK&&a[0].minor==8&&a[1].minor==1&&a[2].minor==1);
    for(size_t i=0;i<4;i++)old[i]=a[i];
    for(size_t cut=0;cut<=3;cut++)CHECK(cm_settle(a,4,0,dest,w,3,m(2),cut)==CM_ABORTED&&!memcmp(a,old,sizeof a));
    dest[1]=1;CHECK(cm_settle(a,4,0,dest,w,3,m(2),SIZE_MAX)==CM_INVALID&&!memcmp(a,old,sizeof a));dest[1]=2;
    CHECK(cm_settle(a,4,0,dest,w,3,m(9),SIZE_MAX)==CM_FUNDS&&!memcmp(a,old,sizeof a));
    a[3]=m(INT64_MAX);for(size_t i=0;i<4;i++)old[i]=a[i];
    CHECK(cm_settle(a,4,0,dest,w,3,m(6),SIZE_MAX)==CM_OVERFLOW&&!memcmp(a,old,sizeof a));
    return 0;
}
int main(int argc,char **argv)
{
    CHECK(argc==2);int s=!strcmp(argv[1],"currency")?currency():!strcmp(argv[1],"rounding")?rounding():!strcmp(argv[1],"allocation")?allocation():!strcmp(argv[1],"settlement")?settlement():1;
    if(!s)printf("CM_CHECK:%s passed\n",argv[1]);
    return s;
}
