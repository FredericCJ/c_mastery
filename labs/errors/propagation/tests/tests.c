#include "propagation.h"
#include "propagation_macro.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ER_CHECK EPR_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static int ordinary(void){
    uint32_t out=91;er_result r=epr_pair_explicit((const unsigned char *)"1:65535",7,&out);CHECK(r.tag==ER_RANGE&&r.as.offset==7&&out==91);
    r=epr_pair_explicit((const unsigned char *)"1:2x",4,&out);CHECK(r.tag==ER_INVALID&&r.as.offset==3&&out==91);
    r=epr_pair_macro((const unsigned char *)"65536:0",7,&out);CHECK(r.tag==ER_RANGE&&r.as.offset==4&&out==91);
    r=epr_pair_macro((const unsigned char *)"1:65534",7,&out);CHECK(r.tag==ER_OK&&out==65535);
    r=epr_pair_explicit(NULL,0,&out);CHECK(r.tag==ER_INVALID&&r.as.offset==0&&out==65535);
    puts("EPR_ORDINARY checked-pieces exact-end atomic-output explicit-and-macro");return 0;
}
static int stream(void){
    EprParser p={0};uint32_t out=91;EprStep r;CHECK(epr_init(&p,1)==ER_OK);
    r=epr_feed(&p,(const unsigned char *)"-12:",4,0,&out);CHECK(r.code==ER_AGAIN&&r.consumed==4&&r.absolute==4&&out==91);
    r=epr_feed(&p,(const unsigned char *)"15x",3,1,&out);CHECK(r.code==ER_INVALID&&r.consumed==2&&r.absolute==6&&r.error==6&&out==91);
    r=epr_feed(&p,(const unsigned char *)"9",1,1,&out);CHECK(r.code==ER_INVALID&&r.consumed==0&&r.error==6&&out==91);
    CHECK(epr_init(&p,1)==ER_OK);r=epr_feed(&p,(const unsigned char *)"-12:11",6,1,&out);CHECK(r.code==ER_RANGE&&r.error==6&&out==91);
    CHECK(epr_init(&p,1)==ER_OK);r=epr_feed(&p,(const unsigned char *)"-",1,0,&out);CHECK(r.code==ER_AGAIN&&r.absolute==1&&out==91);
    r=epr_feed(&p,(const unsigned char *)"65535:65535",11,1,&out);CHECK(r.code==ER_OK&&r.consumed==11&&r.absolute==12&&out==0);
    out=19;r=epr_feed(&p,NULL,0,1,&out);CHECK(r.code==ER_OK&&out==0&&r.consumed==0);
    unsigned char scratch[3]={'-','0',':'};CHECK(epr_init(&p,1)==ER_OK);out=91;
    r=epr_feed(&p,scratch,1,0,&out);CHECK(r.code==ER_AGAIN&&out==91);
    scratch[0]='1';scratch[1]=':';r=epr_feed(&p,scratch,2,0,&out);CHECK(r.code==ER_AGAIN&&out==91);
    scratch[0]='2';r=epr_feed(&p,scratch,1,1,&out);CHECK(r.code==ER_OK&&out==1&&r.absolute==4);
    CHECK(epr_init(&p,0)==ER_OK);p.position=SIZE_MAX-1;p.digits=1;EprParser old=p;out=91;
    r=epr_feed(&p,(const unsigned char *)"00",2,0,&out);CHECK(r.code==ER_RANGE&&r.consumed==0&&p.position==old.position&&p.first==old.first&&p.phase==old.phase&&out==91);
    puts("EPR_STREAM signed-first absolute-errors consumed-prefix final-only nonnegative finite-position");return 0;
}
typedef struct {unsigned expressions,destinations,after;er_code code;er_result result;} Calls;
static er_result counted(Calls *c){++c->expressions;return c->code==ER_OK?er_ok(7):er_error(c->code,4);}
static er_result *destination(Calls *c){++c->destinations;return &c->result;}
static void invocation(Calls *c,unsigned yes){
    if(yes)EPR_TRY(fresh_value,counted(c),destination(c),failed);else c->after=2;
    c->after=1;
failed:return;
}
static int macro(void){
    Calls c={0};c.code=ER_OK;invocation(&c,1);CHECK(c.expressions==1&&c.destinations==1&&c.after==1&&c.result.tag==ER_OK&&c.result.as.value==7);
    c=(Calls){0};c.code=ER_IO;invocation(&c,1);CHECK(c.expressions==1&&c.destinations==1&&c.after==0&&c.result.tag==ER_IO&&c.result.as.offset==4);
    c=(Calls){0};er_result r=epr_identity(counted(&c));CHECK(c.expressions==1&&r.tag==ER_OK);
    uint32_t temporary=13;EPR_TRY(distinct_name,er_ok(temporary),&r,impossible);CHECK(r.as.value==13&&temporary==13);
    puts("EPR_MACRO single-expression single-destination if-else explicit-fresh-name inline-contrast");return 0;
impossible:return 1;
}
static int base(void){uint32_t out=91;er_result r=er_parse_pair((const unsigned char *)"1:",2,&out);CHECK(r.tag==ER_INVALID&&r.as.offset==2&&out==91);r=er_parse_pair((const unsigned char *)"1:2x",4,&out);CHECK(r.tag==ER_INVALID&&r.as.offset==3&&out==91);puts("EPR_BASE actual-parser early-output trailing-input-controls");return 0;}
static int oracle(void){char mode;unsigned signed_first,split,n,octet;unsigned char bytes[512];while(scanf(" %c %u %u %u",&mode,&signed_first,&split,&n)==4){CHECK(n<=512&&split<=n);for(unsigned i=0;i<n;++i){CHECK(scanf("%u",&octet)==1&&octet<=255);bytes[i]=(unsigned char)octet;}uint32_t a=91,b=91,c=91;
    if(mode=='P'){er_result x=er_parse_pair(bytes,n,&a),y=epr_pair_explicit(bytes,n,&b),z=epr_pair_macro(bytes,n,&c);printf("%u %zu %" PRIu32 " %u %zu %" PRIu32 " %u %zu %" PRIu32 "\n",(unsigned)x.tag,x.tag==ER_OK?0:x.as.offset,a,(unsigned)y.tag,y.tag==ER_OK?0:y.as.offset,b,(unsigned)z.tag,z.tag==ER_OK?0:z.as.offset,c);}
    else{EprParser p={0};CHECK(epr_init(&p,signed_first)==ER_OK);EprStep x=epr_feed(&p,bytes,split,0,&a),y=epr_feed(&p,bytes+split,n-split,1,&a);printf("%u %zu %zu %zu %u %zu %zu %zu %" PRIu32 "\n",(unsigned)x.code,x.consumed,x.absolute,x.error,(unsigned)y.code,y.consumed,y.absolute,y.error,a);}
}return 0;}
static int metrics(void){double times[3][7];uint64_t sum=0;for(unsigned k=0;k<3;++k)for(unsigned s=0;s<7;++s){clock_t start=clock();for(unsigned i=0;i<10000;++i){uint32_t out=0;er_result r=k==0?er_parse_pair((const unsigned char *)"123:456",7,&out):k==1?epr_pair_explicit((const unsigned char *)"123:456",7,&out):epr_pair_macro((const unsigned char *)"123:456",7,&out);CHECK(r.tag==ER_OK&&out==579);sum+=out;}times[k][s]=(double)(clock()-start)/CLOCKS_PER_SEC;}printf("{\"iterations\":10000,\"checksum\":%" PRIu64 ",\"parser_size\":%zu,\"parser_align\":%zu,\"allocation_calls\":0",sum,sizeof(EprParser),_Alignof(EprParser));for(unsigned k=0;k<3;++k){printf(",\"%s_seconds\":[",k==0?"base":k==1?"explicit":"macro");for(unsigned s=0;s<7;++s)printf("%s%.9f",s?",":"",times[k][s]);putchar(']');}puts("}");return 0;}
int main(int n,char **v){if(n!=2)return 2;if(!strcmp(v[1],"prop_ordinary"))return ordinary();if(!strcmp(v[1],"prop_stream"))return stream();if(!strcmp(v[1],"prop_macro"))return macro();if(!strcmp(v[1],"prop_base"))return base();if(!strcmp(v[1],"oracle"))return oracle();if(!strcmp(v[1],"metrics"))return metrics();return 2;}
