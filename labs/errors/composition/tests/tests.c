#include "composition.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"ER_CHECK ERC line%d: %s\n",__LINE__,#x);return 1;}}while(0)
struct er_fn {unsigned id,calls,trace;};
static uint32_t pure(uint32_t x,void *p){struct er_fn *f=p;++f->calls;f->trace=f->trace*9+x+1;return f->id<8?(x+f->id)%8:x^(f->id-8);}
static er_result fallible(uint32_t x,void *p){struct er_fn *f=p;++f->calls;f->trace=f->trace*9+x+1;return x>=f->id/8?er_error(ER_RANGE,100+x):er_ok((x+f->id%8)%8);}
struct er_pair {struct er_fn *f,*g;};
static uint32_t combined(uint32_t x,void *p){struct er_pair *q=p;return pure(pure(x,q->f),q->g);}
static er_result bound(uint32_t x,void *p){struct er_pair *q=p;return er_then(fallible(x,q->f),fallible,q->g);}
static int same(er_result a,er_result b){return a.tag==b.tag&&(a.tag==ER_OK?a.as.value==b.as.value:a.as.offset==b.as.offset);}
static size_t payload(er_result r){return r.tag==ER_OK?r.as.value:r.as.offset;}
static er_result input(unsigned tag,unsigned value){er_result r={0};r.tag=(er_code)tag;if(tag==ER_OK)r.as.value=value;else r.as.offset=value;return r;}
static int laws(void){
    for(unsigned a=0;a<16;++a)for(unsigned b=0;b<16;++b)for(unsigned x=0;x<8;++x){
        struct er_fn f={a,0,0},g={b,0,0},h={a,0,0},k={b,0,0},id={0,0,0};struct er_pair p={&h,&k};
        er_result r=er_ok(x),left=er_map(er_map(r,pure,&f),pure,&g),right=er_map(r,combined,&p);
        CHECK(same(left,right)&&same(er_map(r,pure,&id),r));CHECK(f.calls==1&&g.calls==1&&f.calls==h.calls&&g.calls==k.calls&&f.trace==h.trace&&g.trace==k.trace);
    }
    for(unsigned a=0;a<72;++a)for(unsigned b=0;b<72;++b)for(unsigned x=0;x<8;++x){
        struct er_fn f={a,0,0},g={b,0,0},h={a,0,0},k={b,0,0};struct er_pair p={&h,&k};
        er_result r=er_ok(x),left=er_then(er_then(r,fallible,&f),fallible,&g),right=er_then(r,bound,&p);
        CHECK(same(left,right)&&f.calls==1&&f.calls==h.calls&&g.calls==k.calls&&f.trace==h.trace&&g.trace==k.trace);
    }
    puts("ERC_LAWS finite-table map-identity composition bind-associativity exact-callback-traces");return 0;
}
static int base(void){
    struct er_fn f={1,0,0};er_result r=er_error(ER_IO,7);CHECK(same(er_map(r,pure,&f),r)&&f.calls==0);
    CHECK(same(er_then(r,fallible,&f),r)&&f.calls==0);
    r=er_then(er_ok(5),fallible,&f);CHECK(r.tag==ER_RANGE&&r.as.offset==105&&f.calls==1);
    r=er_then(r,fallible,&f);CHECK(r.tag==ER_RANGE&&r.as.offset==105&&f.calls==1);
    CHECK(er_map(er_ok(0),NULL,NULL).tag==ER_INVALID);CHECK(er_then(input(99,7),fallible,&f).tag==ER_INVALID&&f.calls==1);
    puts("ERC_BASE actual-er-map-then active-member preserved-error selected-callback");return 0;
}
static int effects(void){
    struct er_fn lazy={8,0,0},eager={8,0,0};er_result r=er_error(ER_IO,7);
    er_result a=er_then(r,fallible,&lazy),unused=fallible(0,&eager),b=r.tag==ER_OK?unused:r;
    CHECK(same(a,b)&&lazy.calls==0&&eager.calls==1);CHECK(lazy.trace==0&&eager.trace==1);
    struct erc_error e={0};struct erc_result error={0},copy={0};CHECK(erc_cause((struct erc_cause){1,2,7},&e)==ERC_OK);CHECK(erc_error(&e,&error)==ERC_OK);
    CHECK(erc_map_clone(&error,NULL,NULL,&copy)==ERC_OK&&copy.tag==ERC_ERROR&&copy.as.error.cause[0].offset==7);
    CHECK(erc_drop(&copy)==ERC_OK&&erc_drop(&error)==ERC_OK);
    puts("ERC_EFFECTS eager-unused-one lazy-zero equal-values different-effects");return 0;
}
struct er_map {unsigned calls;};
static int32_t flip(int32_t x,void *p){struct er_map *m=p;++m->calls;return x==INT32_MIN?INT32_MAX:-x;}
struct er_watch {struct v4_allocator underlying;struct erc_result *result;unsigned empties;};
static int32_t *take(void *p,size_t n){struct er_watch *w=p;return w->underlying.take(w->underlying.context,n);}
static void release(void *p,int32_t *v){struct er_watch *w=p;if(w->result->tag==ERC_EMPTY)++w->empties;w->underlying.release(w->underlying.context,v);}
static int owner(void){
    struct ow_allocator arena={0};struct eop_owner source={0};struct erc_result r={0},mapped={0};struct er_watch w={ow_allocator(&arena),&r,0};struct v4_allocator a={.context=&w,.take=take,.release=release};
    CHECK(eop_create(&source,a,0,INT32_MIN)==EOP_OK);CHECK(erc_adopt(&source,&r)==ERC_OK&&source.tag==EOP_NONE&&r.tag==ERC_VALUE);
    struct er_map m={0};CHECK(erc_map_clone(&r,flip,&m,&mapped)==ERC_OK&&m.calls==1&&arena.live==2);
    CHECK(r.as.owner.as.some.cell!=mapped.as.owner.as.some.cell&&*r.as.owner.as.some.cell==INT32_MIN&&*mapped.as.owner.as.some.cell==INT32_MAX);
    CHECK(erc_map_clone(&r,flip,&m,&mapped)==ERC_BUSY&&m.calls==1);
    CHECK(erc_map_clone(&r,flip,&m,&r)==ERC_INVALID&&m.calls==1);
    CHECK(erc_drop(&r)==ERC_OK&&w.empties==1&&arena.live==1);w.result=&mapped;
    CHECK(erc_drop(&mapped)==ERC_OK&&w.empties==2&&arena.live==0&&arena.releases==2);CHECK(erc_drop(&mapped)==ERC_OK&&arena.releases==2);
    puts("ERC_OWNER actual-allocator clone-distinct borrowed-input move-empty drop-before-callback");return 0;
}
static int failure(void){
    struct ow_allocator arena={0};struct eop_owner source={0};struct erc_result r={0},out={0};struct er_map m={0};
    CHECK(eop_create(&source,ow_allocator(&arena),0,19)==EOP_OK);CHECK(erc_adopt(&source,&r)==ERC_OK);
    arena.fail_at=2;CHECK(erc_map_clone(&r,flip,&m,&out)==ERC_NOMEM&&out.tag==ERC_EMPTY&&m.calls==0&&arena.live==1&&*r.as.owner.as.some.cell==19);
    CHECK(erc_map_clone(&r,NULL,&m,&out)==ERC_INVALID&&arena.attempts==2);
    struct erc_error e={0};CHECK(erc_cause((struct erc_cause){2,3,SIZE_MAX},&e)==ERC_OK);CHECK(erc_error(&e,&out)==ERC_OK);
    struct erc_result copied={0};CHECK(erc_map_clone(&out,flip,&m,&copied)==ERC_OK&&m.calls==0&&arena.attempts==2);
    CHECK(copied.as.error.cause[0].offset==SIZE_MAX);CHECK(erc_drop(&copied)==ERC_OK&&erc_drop(&out)==ERC_OK&&erc_drop(&r)==ERC_OK&&arena.live==0&&arena.releases==1);
    puts("ERC_FAILURE clone-admission-before-callback no-owner-publication no-error-callback");return 0;
}
static int cause(void){
    struct erc_error e={0},out={0};CHECK(erc_cause((struct erc_cause){1,2,SIZE_MAX},&e)==ERC_OK);
    for(unsigned n=1;n<4;++n){CHECK(erc_wrap(&e,(struct erc_cause){n+1,7,n},&out)==ERC_OK);CHECK(out.count==n+1&&out.cause[0].offset==SIZE_MAX&&out.cause[0].code==2);e=out;}
    CHECK(erc_wrap(&e,(struct erc_cause){5,8,9},&out)==ERC_FULL&&out.count==4&&out.cause[3].domain==4);
    CHECK(erc_wrap(&e,(struct erc_cause){0,8,9},&out)==ERC_INVALID&&out.count==4);
    e.count=0;CHECK(erc_wrap(&e,(struct erc_cause){2,3,4},&out)==ERC_INVALID&&out.count==4);
    puts("ERC_CAUSE bounded-inner-outer exact-offset no-allocation full-refusal-no-truncation");return 0;
}
static int oracle(void){
    char kind;unsigned a,b,tag,x;
    while(scanf(" %c %u %u %u %u",&kind,&a,&b,&tag,&x)==5){
        if(kind=='M'||kind=='B'){
            struct er_fn f={a,0,0},g={b,0,0},h={a,0,0},k={b,0,0};struct er_pair p={&h,&k};er_result r=input(tag,x),l,v;
            if(kind=='M'){l=er_map(er_map(r,pure,&f),pure,&g);v=er_map(r,combined,&p);}else{l=er_then(er_then(r,fallible,&f),fallible,&g);v=er_then(r,bound,&p);}
            printf("%d %zu %u %u %u %u %d %zu %u %u %u %u\n",l.tag,payload(l),f.calls,g.calls,f.trace,g.trace,v.tag,payload(v),h.calls,k.calls,h.trace,k.trace);
        }else if(kind=='W'){
            struct erc_error e={0},out={0};e.count=a;for(unsigned i=0;i<a&&i<4;++i)e.cause[i]=(struct erc_cause){1+i,2+i,x+i};out.count=99;out.cause[0]=(struct erc_cause){91,92,93};
            int c=erc_wrap(&e,(struct erc_cause){b,tag,77},&out);printf("%d %u %u %u %zu %u %zu\n",c,out.count,out.cause[0].domain,out.cause[0].code,out.cause[0].offset,c==ERC_OK?out.cause[out.count-1].domain:0,c==ERC_OK?out.cause[out.count-1].offset:0);
        }else if(kind=='O'){
            struct ow_allocator arena={0};arena.fail_at=a;struct eop_owner source={0};struct erc_result r={0},o={0};struct er_map m={0};int s=eop_create(&source,ow_allocator(&arena),0,(int32_t)x),c=ERC_INVALID;
            if(s==EOP_OK){if(erc_adopt(&source,&r)!=ERC_OK)return 2;c=erc_map_clone(&r,flip,&m,&o);}
            int32_t v=o.tag==ERC_VALUE?*o.as.owner.as.some.cell:91;size_t live=arena.live;
            if(erc_drop(&r)!=ERC_OK||erc_drop(&o)!=ERC_OK)return 2;
            printf("%d %d %u %d %zu %zu %zu %zu\n",s,c,m.calls,v,live,arena.attempts,arena.releases,arena.live);
        }else return 2;
    }
    return ferror(stdin)?2:0;
}
static int metrics(void){
    double scalar[7],owned[7];unsigned long checksum=0;size_t allocations=0,releases=0;
    for(unsigned sample=0;sample<7;++sample){clock_t start=clock();for(unsigned i=0;i<10000;++i){struct er_fn f={65,0,0},g={66,0,0};er_result r=er_then(er_then(er_ok(i%8),fallible,&f),fallible,&g);CHECK(r.tag==ER_OK);checksum+=r.as.value;}scalar[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;
        start=clock();for(unsigned i=0;i<10000;++i){struct ow_allocator arena={0};struct eop_owner o={0};struct erc_result r={0},out={0};struct er_map m={0};CHECK(eop_create(&o,ow_allocator(&arena),0,7)==EOP_OK&&erc_adopt(&o,&r)==ERC_OK&&erc_map_clone(&r,flip,&m,&out)==ERC_OK);checksum+=(unsigned)(-*out.as.owner.as.some.cell);CHECK(erc_drop(&r)==ERC_OK&&erc_drop(&out)==ERC_OK);allocations+=arena.attempts;releases+=arena.releases;}owned[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;}
    printf("{\"iterations\":10000,\"checksum\":%lu,\"allocation_calls\":%zu,\"release_calls\":%zu,\"result_size\":%zu,\"result_align\":%zu,\"error_size\":%zu,\"scalar_seconds\":[",checksum,allocations,releases,sizeof(struct erc_result),_Alignof(struct erc_result),sizeof(struct erc_error));
    for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",scalar[i]);
    printf("],\"owned_seconds\":[");for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",owned[i]);
    puts("]}");return 0;
}
int main(int argc,char **argv){
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"compose_laws"))return laws();
    if(!strcmp(argv[1],"compose_base"))return base();
    if(!strcmp(argv[1],"compose_effects"))return effects();
    if(!strcmp(argv[1],"compose_owner"))return owner();
    if(!strcmp(argv[1],"compose_failure"))return failure();
    if(!strcmp(argv[1],"compose_cause"))return cause();
    if(!strcmp(argv[1],"oracle"))return oracle();
    if(!strcmp(argv[1],"metrics"))return metrics();
    return 2;
}
