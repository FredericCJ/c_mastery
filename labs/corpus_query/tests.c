#include "query.h"
#include <stdio.h>
#include <string.h>
#define CHECK(v) do{if(!(v)){fprintf(stderr,"QI_CHECK:%d\n",__LINE__);return 1;}}while(0)
static struct qi_row row(unsigned alive,unsigned known,int value,unsigned a,unsigned b)
{struct qi_row r={0};r.alive=(uint8_t)alive;r.has_value=(uint8_t)known;r.value=(int16_t)value;r.length=2;r.token[0]=(uint8_t)a;r.token[1]=(uint8_t)b;return r;}
static int bitmap(void)
{
    struct qi_row rows[32];struct qi_index x;uint32_t answer=99;
    for(unsigned i=0;i<32;i++)rows[i]=row(i%3!=0,i%2,(int)i,i%8,(i+1)%8);
    CHECK(qi_build(rows,32,&x)==QI_OK);CHECK(qi_terms(&x,0,0,&answer)==QI_OK&&answer==x.live);
    CHECK(qi_terms(&x,1,0,&answer)==QI_OK);uint32_t before=answer;
    CHECK(qi_replace(&x,7,row(1,1,7,2,3),0)==QI_OK);
    CHECK(qi_terms(&x,1,0,&answer)==QI_OK&&answer==(before&~(UINT32_C(1)<<7)));
    for(unsigned a=0;a<3;a++)for(unsigned b=0;b<3;b++){
        struct qi_truth ta={a==1?2u:0,a==0?2u:0},tb={b==1?2u:0,b==0?2u:0},out;
        CHECK(qi_logic(&x,ta,tb,0,&out)==QI_OK);
        CHECK(out.yes==(a==1&&b==1?2u:0)&&out.no==(a==0||b==0?2u:0));
        CHECK(qi_logic(&x,ta,tb,1,&out)==QI_OK);
        CHECK(out.yes==(a==1||b==1?2u:0)&&out.no==(a==0&&b==0?2u:0));
        CHECK(qi_logic(&x,ta,tb,2,&out)==QI_OK&&out.yes==ta.no&&out.no==ta.yes);
    }
    struct qi_truth truth={99,77};CHECK(qi_compare(&x,15,&truth)==QI_OK);
    for(unsigned i=0;i<32;i++){
        uint32_t bit=UINT32_C(1)<<i;
        CHECK(!!(truth.yes&bit)==!!(x.row[i].alive&&x.row[i].has_value&&x.row[i].value>=15));
        CHECK(!!(truth.no&bit)==!!(x.row[i].alive&&x.row[i].has_value&&x.row[i].value<15));
    }
    struct qi_truth invalid={2,2},sentinel={3,4};CHECK(qi_logic(&x,invalid,truth,0,&sentinel)==QI_INVALID&&sentinel.yes==3&&sentinel.no==4);
    CHECK(qi_build(NULL,0,&x)==QI_OK&&qi_terms(&x,0,0,&answer)==QI_OK&&answer==0);
    CHECK(qi_build(rows,33,&x)==QI_INVALID);return 0;
}
static int postings(void)
{
    struct qi_row rows[]={row(1,1,0,1,0),row(1,1,1,0,1),row(1,1,2,1,1)};
    struct qi_index x;uint32_t answer=99;uint8_t phrase[]={0,1};
    CHECK(qi_build(rows,3,&x)==QI_OK);
    CHECK(qi_phrase(&x,phrase,2,&answer)==QI_OK&&answer==2);
    CHECK(qi_near(&x,0,0,1,&answer)==QI_OK&&answer==0);
    CHECK(qi_near(&x,1,1,1,&answer)==QI_OK&&answer==4);
    phrase[0]=phrase[1]=1;CHECK(qi_phrase(&x,phrase,2,&answer)==QI_OK&&answer==4);
    CHECK(qi_phrase(&x,NULL,0,&answer)==QI_OK&&answer==7);
    CHECK(qi_near(&x,1,1,0,&answer)==QI_OK&&answer==0);
    CHECK(qi_replace(&x,2,row(0,1,0,1,1),0)==QI_OK);
    CHECK(qi_phrase(&x,phrase,2,&answer)==QI_OK&&answer==0);
    phrase[0]=8;answer=99;CHECK(qi_phrase(&x,phrase,2,&answer)==QI_INVALID&&answer==99);
    CHECK(qi_phrase(&x,phrase,9,&answer)==QI_INVALID&&answer==99);
    for(unsigned t=0;t<8;t++)for(unsigned i=1;i<x.posting_count[t];i++)CHECK(x.postings[t][i-1].row<x.postings[t][i].row);
    return 0;
}
static int zones(void)
{
    struct qi_row rows[32];struct qi_index x;uint32_t answer;struct qi_cost cost;
    for(unsigned i=0;i<32;i++)rows[i]=row(1,1,(int)i,0,1);
    CHECK(qi_build(rows,32,&x)==QI_OK);
    CHECK(qi_range(&x,4,4,&answer,&cost)==QI_OK&&answer==16&&cost.groups==1&&cost.rows==4);
    CHECK(qi_replace(&x,1,row(1,1,100,0,1),1)==QI_OK);
    CHECK(qi_range(&x,100,100,&answer,&cost)==QI_OK&&answer==2);
    CHECK(qi_replace(&x,1,row(0,1,100,0,1),1)==QI_OK);
    CHECK(qi_range(&x,100,100,&answer,&cost)==QI_OK&&answer==0&&cost.groups==1);
    CHECK(qi_rebuild_zones(&x)==QI_OK);
    CHECK(qi_range(&x,100,100,&answer,&cost)==QI_OK&&answer==0&&cost.groups==0);
    struct qi_index before=x;struct qi_row bad=row(1,1,0,1,2);bad.length=9;
    CHECK(qi_replace(&x,0,bad,0)==QI_INVALID&&!memcmp(&x,&before,sizeof x));
    answer=77;cost=(struct qi_cost){8,9};CHECK(qi_range(&x,4,3,&answer,&cost)==QI_INVALID&&answer==77&&cost.groups==8&&cost.rows==9);
    uint8_t phrase[]={0,1};CHECK(qi_search(&x,1,0,4,7,phrase,2,&answer,&cost)==QI_OK&&answer==240);
    phrase[1]=8;answer=77;cost=(struct qi_cost){8,9};CHECK(qi_search(&x,1,0,4,7,phrase,2,&answer,&cost)==QI_INVALID&&answer==77&&cost.rows==9);
    for(unsigned i=0;i<32;i++)rows[i].has_value=0;
    CHECK(qi_build(rows,32,&x)==QI_OK&&qi_range(&x,INT16_MIN,INT16_MAX,&answer,&cost)==QI_OK&&answer==0&&cost.rows==0);
    return 0;
}
int main(int argc,char **argv)
{
    CHECK(argc==2);int status=!strcmp(argv[1],"bitmap")?bitmap():!strcmp(argv[1],"postings")?postings():!strcmp(argv[1],"zones")?zones():1;
    if(!status)printf("QI_CHECK:%s passed\n",argv[1]);
    return status;
}
