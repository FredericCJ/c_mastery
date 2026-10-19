#include "test_support.h"
#include <time.h>
static int values(void) {
    Counter c={0};AdoValue v;ado_init(&v);uint64_t n=99;uint32_t u=99;
    CHECK(ado_measure(&v,&n)==ADO_OK&&n==0);
    CHECK(ado_from_value(&v,(ADValue){AD_PAIR,{.pair={UINT32_MAX,UINT32_MAX}}})==ADO_OK);
    CHECK(ado_measure(&v,&n)==ADO_OK&&n==UINT64_C(8589934590));
    CHECK(ado_number(&v,&u)==ADO_INVALID&&u==99);
    CHECK(ado_from_value(&v,(ADValue){AD_NUMBER,{.number=0}})==ADO_OK);
    CHECK(ado_number(&v,&u)==ADO_OK&&u==0);
    const unsigned char raw[]={0,'a',255};
    CHECK(ado_text(&v,raw,3,operations(&c))==ADO_OK);
    CHECK(ado_measure(&v,&n)==ADO_OK&&n==3);
    u=99;CHECK(ado_number(&v,&u)==ADO_INVALID&&u==99);
    CHECK(ado_from_value(&v,(ADValue){(ADTag)99,{.number=7}})==ADO_INVALID&&v.tag==ADO_TEXT);
    CHECK(ado_text(&v,NULL,0,operations(&c))==ADO_OK&&v.tag==ADO_TEXT&&c.live==1);
    CHECK(ado_measure(&v,&n)==ADO_OK&&n==0);ado_drop(&v);CHECK(c.live==0);
    puts("ADO_VALUES variants4 embedded-nul binary-text partial-elimination");return 0;
}
static int owner(void) {
    Counter a={0},b={0};AdoValue x,y;ado_init(&x);ado_init(&y);
    unsigned char raw[]={1,2,3};CHECK(ado_text(&x,raw,3,operations(&a))==ADO_OK);raw[0]=9;
    CHECK(x.data.text.bytes[0]==1);
    CHECK(ado_text(&y,raw,3,operations(&b))==ADO_OK);
    CHECK(ado_copy(&y,&x)==ADO_OK);
    CHECK(x.data.text.bytes!=y.data.text.bytes&&a.live==2&&b.live==0&&b.drops==1);
    CHECK(y.data.text.bytes[0]==1);
    CHECK(ado_copy(&x,&x)==ADO_OK&&a.calls==2);
    CHECK(ado_move(&x,&x)==ADO_OK&&a.live==2);
    CHECK(ado_move(&y,&x)==ADO_OK&&x.tag==ADO_EMPTY&&a.live==1);
    ado_drop(&x);ado_drop(&y);ado_drop(&y);CHECK(a.live==0&&a.drops==2);
    puts("ADO_OWNER independent-copy policy-transfer self-operation release-once");return 0;
}
static int rollback(void) {
    for(unsigned partial=0;partial<2;++partial) {
        Counter c={0};c.partial=partial;AdoValue x,y;ado_init(&x);ado_init(&y);
        unsigned char a[]={1,2},b[]={3,4,5};
        CHECK(ado_text(&x,a,2,operations(&c))==ADO_OK&&ado_text(&y,b,3,operations(&c))==ADO_OK);
        unsigned char *prior=y.data.text.bytes;c.fail=c.calls+1;
        CHECK(ado_copy(&y,&x)==ADO_NOMEM);
        CHECK(y.tag==ADO_TEXT&&y.data.text.bytes==prior&&y.data.text.length==3&&c.live==2&&c.drops==0);
        CHECK(c.partial_cleanups==partial);c.fail=0;
        CHECK(ado_text(&y,y.data.text.bytes+1,2,operations(&c))==ADO_OK);
        CHECK(y.data.text.length==2&&y.data.text.bytes[0]==4&&y.data.text.bytes[1]==5&&c.live==2);
        CHECK(ado_text(&y,NULL,1,operations(&c))==ADO_INVALID);
        CHECK(ado_text(&y,a,65536,operations(&c))==ADO_INVALID);
        ado_drop(&x);ado_drop(&y);CHECK(c.live==0&&c.drops==3);
    }
    puts("ADO_ROLLBACK before-allocation after-allocation internal-borrow staged-publish");return 0;
}
static int base(void) {
    uint64_t n=99;uint32_t u=99;
    CHECK(ad_measure((ADValue){AD_PAIR,{.pair={UINT32_MAX,5}}},&n)&&n==UINT64_C(4294967300));
    CHECK(!ad_measure((ADValue){(ADTag)99,{.number=0}},&n)&&n==UINT64_C(4294967300));
    CHECK(!ad_number((ADValue){AD_PAIR,{.pair={1,2}}},&u)&&u==99);
    puts("ADO_BASE actual-ad-measure and-number");return 0;
}
static int trace(const char *word) {
    Counter c={0};AdoValue v[2];ado_init(v);ado_init(v+1);
    const unsigned char text[]={0,17,255};
    for(size_t i=0;word[i];++i) {
        AdoStatus s=ADO_OK;c.fail=0;
        switch(word[i]) {
        case '0':s=ado_from_value(v,(ADValue){AD_NUMBER,{.number=0}});break;
        case '1':s=ado_from_value(v,(ADValue){AD_PAIR,{.pair={UINT32_MAX,5}}});break;
        case '2':s=ado_text(v,text,3,operations(&c));break;
        case '3':s=ado_text(v,NULL,0,operations(&c));break;
        case '4':s=ado_copy(v+1,v);break;
        case '5':s=ado_move(v+1,v);break;
        case '6':ado_drop(v);break;
        case '7':ado_drop(v+1);break;
        case '8':c.fail=c.calls+1;s=ado_copy(v+1,v);break;
        case '9':s=ado_copy(v,v+1);break;
        default:return 1;
        }
        uint64_t a=99,b=99;CHECK(ado_measure(v,&a)==ADO_OK&&ado_measure(v+1,&b)==ADO_OK);
        printf("%u %u %llu %u %llu %zu %zu %zu;",(unsigned)s,(unsigned)v[0].tag,(unsigned long long)a,(unsigned)v[1].tag,(unsigned long long)b,c.calls,c.drops,c.live);
    }
    ado_drop(v);ado_drop(v+1);CHECK(c.live==0);puts("");return 0;
}
int main(int argc,char **argv) {
    if(atexit(cleanup_allocations))return 2;
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"sum_value"))return values();
    if(!strcmp(argv[1],"sum_owner"))return owner();
    if(!strcmp(argv[1],"sum_rollback"))return rollback();
    if(!strcmp(argv[1],"sum_base"))return base();
    if(!strcmp(argv[1],"oracle")){char line[80];while(fgets(line,sizeof line,stdin)){line[strcspn(line,"\r\n")]=0;if(trace(line))return 1;}return 0;}
    if(!strcmp(argv[1],"metrics")){
        Counter c={0};AdoValue v,w;ado_init(&v);ado_init(&w);const unsigned char p[]={1,2,3};double samples[7];
        for(unsigned k=0;k<7;++k){clock_t begin=clock();for(unsigned i=0;i<10000;++i){CHECK(ado_text(&v,p,3,operations(&c))==ADO_OK);CHECK(ado_copy(&w,&v)==ADO_OK);ado_drop(&v);ado_drop(&w);}samples[k]=(double)(clock()-begin)/CLOCKS_PER_SEC;}
        CHECK(c.live==0);printf("{\"value_size\":%zu,\"tag_size\":%zu,\"iterations\":10000,\"samples\":[",sizeof(AdoValue),sizeof(AdoTag));for(unsigned k=0;k<7;++k)printf("%s%.9f",k?",":"",samples[k]);puts("]}");return 0;
    }
    return 2;
}
