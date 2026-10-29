#include "partial_optics.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ADO_CHECK %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
static struct adl_config config(unsigned p,unsigned q,uint32_t x,uint32_t y){return (struct adl_config){p,{q,{x,y},17},29,41};}
static int equal(struct adl_config a,struct adl_config b){return a.present==b.present&&a.inner.present==b.inner.present&&a.inner.endpoint.x==b.inner.endpoint.x&&a.inner.endpoint.y==b.inner.endpoint.y&&a.inner.audit==b.inner.audit&&a.identity==b.identity&&a.footer==b.footer;}
static int ioeq(struct adl_io a,struct adl_io b){return a.version==b.version&&a.code==b.code&&a.context==b.context;}
static int erroreq(struct adl_error a,struct adl_error b){return a.version==b.version&&a.tag==b.tag&&(a.tag==ADL_SUCCESS||(a.tag==ADL_IO?(a.data.io.x==b.data.io.x&&a.data.io.y==b.data.io.y):a.data.timeout==b.data.timeout));}
static int nested(void){
    for(uint32_t x=0;x<4;x++)for(uint32_t y=0;y<4;y++)for(uint32_t v=0;v<4;v++)for(uint32_t w=0;w<4;w++){
        struct adl_config a=config(1,1,x,y),b,c,d;uint32_t got=99;
        CHECK(adl_get(a,&got)==ADL_OK&&got==x);CHECK(adl_put(a,got,&b)==ADL_OK&&equal(a,b));
        CHECK(adl_put(a,v,&b)==ADL_OK&&adl_get(b,&got)==ADL_OK&&got==v);
        CHECK(b.inner.endpoint.y==y&&b.inner.audit==17&&b.identity==29&&b.footer==41);
        CHECK(adl_put(b,w,&c)==ADL_OK&&adl_put(a,w,&d)==ADL_OK&&equal(c,d));
    }
    for(unsigned p=0;p<2;p++)for(unsigned q=0;q<2;q++)if(!p||!q){struct adl_config a=config(p,q,6,8),b=config(1,1,99,88),saved=b;uint32_t got=71;CHECK(adl_get(a,&got)==ADL_ABSENT&&got==71);CHECK(adl_put(a,9,&b)==ADL_ABSENT&&equal(b,saved));}
    struct adl_config a=config(1,1,UINT32_MAX,UINT32_MAX),b;CHECK(adl_put(a,0,&b)==ADL_OK&&b.inner.endpoint.y==UINT32_MAX);a.present=2;b=config(0,0,51,52);struct adl_config saved=b;CHECK(adl_put(a,1,&b)==ADL_INVALID&&equal(b,saved));
    a=config(0,0,0,0);CHECK(adl_put(a,42,&b)==ADL_ABSENT&&equal(b,saved));
    a=config(1,1,0,11);CHECK(adl_put(a,42,&b)==ADL_OK&&b.inner.endpoint.x==42&&b.inner.endpoint.y==11);
    /* The original clamping specimen has defined arithmetic and fails put-get. */
    struct ad_pair clamped=ad_lens_x((struct ad_pair){1,2},9>3?3:9);CHECK(clamped.x!=9);
    puts("ADL_NESTED partial-composition three-laws collateral-preserved");return 0;
}
static int prism(void){
    const uint32_t codes[]={1,255,256,65535,65536,UINT32_MAX};
    for(unsigned version=1;version<=2;version++)for(size_t i=0;i<6;i++){
        struct adl_io focus={version,codes[i],version==1?255:UINT32_MAX},got={9,8,7};struct adl_error error={2,ADL_IO,{{77,88}}},saved=error;
        int valid=codes[i]<=(version==1?255u:65535u);enum adl_status s=adl_review(focus,&error);
        CHECK(s==(valid?ADL_OK:ADL_RANGE));if(!valid){CHECK(erroreq(error,saved));continue;}
        CHECK(adl_preview(error,&got)==ADL_OK&&ioeq(got,focus));struct adl_error again;CHECK(adl_review(got,&again)==ADL_OK&&erroreq(again,error));
    }
    struct adl_io out={9,8,7},saved=out;struct adl_error timeout={2,ADL_TIMEOUT,{.timeout=50}};CHECK(adl_preview(timeout,&out)==ADL_ABSENT&&ioeq(out,saved));
    struct adl_error success={1,ADL_SUCCESS,{{0,0}}};CHECK(adl_preview(success,&out)==ADL_ABSENT&&ioeq(out,saved));
    timeout.version=1;CHECK(adl_preview(timeout,&out)==ADL_INVALID&&ioeq(out,saved));timeout.version=3;CHECK(adl_preview(timeout,&out)==ADL_VERSION&&ioeq(out,saved));
    struct adl_error original={2,ADL_IO,{{3,4}}},lossy={1,ADL_IO,{{3,0}}};CHECK(!erroreq(original,lossy));
    puts("ADL_PRISM version-and-payload roundtrip partial-domain");return 0;
}
struct adp_change {unsigned mode,calls;uint32_t argument,seen[4];};
static int change(void *context,uint32_t old,uint32_t *out){
    struct adp_change *c=context;if(c->calls<4)c->seen[c->calls]=old;c->calls++;
    if(c->mode==0){*out=old;return 0;}
    if(c->mode==1){if(old>UINT32_MAX-c->argument){*out=123;return 1;}*out=old+c->argument;return 0;}
    if(c->mode==2){*out=c->argument;return 1;}
    *out=c->argument;return 0;
}
static int effects(void){
    struct adl_config a=config(1,1,7,8),out=config(0,0,90,91),saved=out;
    struct adp_change c={0,0,0,{0}},d={2,0,99,{0}};struct adl_step steps[2]={{change,&c},{change,&d}};
    CHECK(adl_update(a,steps,1,&out)==ADL_OK&&equal(out,a)&&c.calls==1&&c.seen[0]==7);
    /* Identity value update still changes the external callback trace. */
    CHECK(c.calls!=0);c=(struct adp_change){1,0,3,{0}};out=saved;
    CHECK(adl_update(a,steps,2,&out)==ADL_CALLBACK&&equal(out,saved)&&c.calls==1&&d.calls==1&&d.seen[0]==10);
    c.calls=0;d.calls=0;a.present=0;CHECK(adl_update(a,steps,2,&out)==ADL_ABSENT&&equal(out,saved)&&c.calls==0&&d.calls==0);
    a.present=1;steps[1].change=NULL;CHECK(adl_update(a,steps,2,&out)==ADL_INVALID&&c.calls==0&&equal(out,saved));
    CHECK(adl_update(a,NULL,0,&out)==ADL_OK&&equal(out,a));
    c=(struct adp_change){1,0,1,{0}};a.inner.endpoint.x=UINT32_MAX;out=saved;CHECK(adl_update(a,steps,1,&out)==ADL_CALLBACK&&equal(out,saved)&&c.calls==1);
    puts("ADL_EFFECT once-in-order external-effects-not-rollback");return 0;
}
static int baseline(void){
    for(uint32_t x=0;x<4;x++)for(uint32_t y=0;y<4;y++)for(uint32_t a=0;a<4;a++)for(uint32_t b=0;b<4;b++){
        struct ad_pair s={x,y},p=ad_lens_x(s,a),q=ad_lens_x(p,b),r=ad_lens_x(s,b);CHECK(p.x==a&&p.y==y&&q.x==r.x&&q.y==r.y);p=ad_lens_x(s,x);CHECK(p.x==x&&p.y==y);
        uint32_t got=99;CHECK(ad_preview_number(ad_review_number(a),&got)&&got==a);
    }
    struct ad_value empty={.tag=AD_EMPTY};uint32_t got=99;CHECK(!ad_preview_number(empty,&got)&&got==99);struct ad_value pair={.tag=AD_PAIR,.data.pair={1,2}};CHECK(!ad_preview_number(pair,&got)&&got==99);
    puts("ADL_BASE actual-lens-prism laws");return 0;
}
static void printconfig(struct adl_config s){printf(" %u %u %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32" %"PRIu32,s.present,s.inner.present,s.inner.endpoint.x,s.inner.endpoint.y,s.inner.audit,s.identity,s.footer);}
static int nested_oracle(void){
    unsigned p,q;uint32_t x,y,a,id,f,v,w;
    while(scanf("%u %u %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32,&p,&q,&x,&y,&a,&id,&f,&v,&w)==9){
        struct adl_config s={p,{q,{x,y},a},id,f},out=config(0,0,81,82),twice=out;uint32_t got=99;enum adl_status g=adl_get(s,&got),put=adl_put(s,v,&out);enum adl_status second=put==ADL_OK?adl_put(out,w,&twice):ADL_INVALID;
        printf("%d %"PRIu32" %d",g,got,put);printconfig(out);printf(" %d",second);printconfig(twice);putchar('\n');
    }return ferror(stdin)?1:0;
}
static int prism_oracle(void){
    unsigned version,tag;uint32_t code,context;
    while(scanf("%u %u %"SCNu32" %"SCNu32,&version,&tag,&code,&context)==4){
        struct adl_error error={version,tag,{{code,context}}},rebuilt={9,ADL_IO,{{81,82}}};if(tag==ADL_TIMEOUT)error.data.timeout=code;
        struct adl_io focus={9,71,72},input={version,code,context};enum adl_status preview=adl_preview(error,&focus),review=adl_review(input,&rebuilt);
        printf("%d %u %"PRIu32" %"PRIu32" %d %u %u %"PRIu32" %"PRIu32"\n",preview,focus.version,focus.code,focus.context,review,rebuilt.version,rebuilt.tag,rebuilt.data.io.x,rebuilt.data.io.y);
    }return ferror(stdin)?1:0;
}
static int effects_oracle(void){
    unsigned p,q,n,m1,m2;uint32_t x,a1,a2;
    while(scanf("%u %u %"SCNu32" %u %u %"SCNu32" %u %"SCNu32,&p,&q,&x,&n,&m1,&a1,&m2,&a2)==8){
        struct adp_change c={m1,0,a1,{0}},d={m2,0,a2,{0}};struct adl_step steps[2]={{change,&c},{change,&d}};struct adl_config out=config(0,0,81,82);enum adl_status s=adl_update(config(p,q,x,12),steps,n,&out);
        printf("%d %u %u %"PRIu32" %"PRIu32,s,c.calls,d.calls,c.seen[0],d.seen[0]);printconfig(out);putchar('\n');
    }return ferror(stdin)?1:0;
}
static int metrics(void){
    double direct[7],composed[7],callbacks[7];uint64_t checksum=0;const unsigned iterations=100000;volatile uint32_t seed=13;struct adl_config source=config(1,1,13,17),out;struct adp_change c={1,0,1,{0}};struct adl_step step={change,&c};
    for(unsigned sample=0;sample<7;sample++){
        clock_t start=clock();checksum=0;for(unsigned i=0;i<iterations;i++){out=source;out.inner.endpoint.x=seed;checksum+=out.inner.endpoint.x+out.inner.endpoint.y;}direct[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;
        start=clock();checksum=0;for(unsigned i=0;i<iterations;i++){CHECK(adl_put(source,seed,&out)==ADL_OK);checksum+=out.inner.endpoint.x+out.inner.endpoint.y;}composed[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;CHECK(checksum==3000000);
        start=clock();checksum=0;c.calls=0;for(unsigned i=0;i<iterations;i++){CHECK(adl_update(source,&step,1,&out)==ADL_OK);checksum+=out.inner.endpoint.x+out.inner.endpoint.y;}callbacks[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;CHECK(checksum==3100000&&c.calls==iterations);
    }
    printf("{\"iterations\":%u,\"checksum\":%"PRIu64",\"callback_calls\":%u,\"config_size\":%zu,\"config_alignment\":%zu,\"error_size\":%zu,\"direct_seconds\":[",iterations,checksum,c.calls,sizeof(struct adl_config),_Alignof(struct adl_config),sizeof(struct adl_error));
    for(unsigned i=0;i<7;i++){printf("%s%.9f",i?",":"",direct[i]);}
    printf("],\"composed_seconds\":[");
    for(unsigned i=0;i<7;i++){printf("%s%.9f",i?",":"",composed[i]);}
    printf("],\"callback_seconds\":[");
    for(unsigned i=0;i<7;i++){printf("%s%.9f",i?",":"",callbacks[i]);}
    puts("]}");return 0;
}
int main(int argc,char **argv){
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"optics_nested"))return nested();
    if(!strcmp(argv[1],"optics_prism"))return prism();
    if(!strcmp(argv[1],"optics_effects"))return effects();
    if(!strcmp(argv[1],"optics_base"))return baseline();
    if(!strcmp(argv[1],"nested_oracle"))return nested_oracle();
    if(!strcmp(argv[1],"prism_oracle"))return prism_oracle();
    if(!strcmp(argv[1],"effects_oracle"))return effects_oracle();
    if(!strcmp(argv[1],"metrics"))return metrics();
    return 2;
}
