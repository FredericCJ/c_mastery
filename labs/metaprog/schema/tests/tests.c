#include "schema.h"
#include "metaprog.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MPO_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
struct mp_visits { unsigned calls,stop;int64_t sum;unsigned tags; };
static int visit(void *p,const struct mps_field *f,int64_t value){struct mp_visits *v=p;++v->calls;v->sum+=value;v->tags=v->tags*10+f->tag;return v->calls!=v->stop;}
static int same(struct mps_record a,struct mps_record b){return a.present==b.present&&a.code==b.code&&a.value==b.value&&a.note==b.note;}
static int wire(void){
    struct mps_record a={7,0x1234,0x89abcdef,-2},b={3,99,88,77},saved=b;unsigned char bytes[64],old[64];size_t n=99,i;unsigned char expected[]={2,3,1,2,0x12,0x34,2,4,0x89,0xab,0xcd,0xef,3,5,0xff,0xff,0xff,0xfe,1};
    memset(bytes,0x55,sizeof bytes);CHECK(mps_encode(2,&a,bytes,sizeof bytes,&n)==MPS_OK&&n==sizeof expected&&!memcmp(bytes,expected,n));
    CHECK(mps_decode(bytes,n,&b)==MPS_OK&&same(a,b));
    for(i=0;i<n;++i){b=saved;CHECK(mps_decode(bytes,i,&b)!=MPS_OK&&same(b,saved));}
    b=saved;bytes[n-1]^=1;CHECK(mps_decode(bytes,n,&b)==MPS_INVALID&&same(b,saved));bytes[n-1]^=1;
    memcpy(old,bytes,sizeof bytes);n=99;CHECK(mps_encode(2,&a,bytes,18,&n)==MPS_FULL&&n==99&&!memcmp(old,bytes,64));
    CHECK(mps_encode(1,&a,bytes,64,&n)==MPS_RANGE&&n==99&&!memcmp(old,bytes,64));
    a.present=3;CHECK(mps_encode(1,&a,bytes,64,&n)==MPS_OK&&n==12);CHECK(mps_decode(bytes,n,&b)==MPS_OK&&b.present==3&&b.note==0);
    CHECK(mps_encode(2,&b,bytes,64,&n)==MPS_OK&&bytes[0]==2&&n==12);MPS_LINK_GUARD();
    puts("MPS_WIRE actual-codec exact-tag-order staged-publication migration");return 0;
}
static int semantics(void){
    struct mps_record a={7,1,2,-3},b=a;unsigned equal=99;const struct mps_field *f=NULL;
    CHECK(mps_fields(&f)==3&&f&&mps_fields(NULL)==0);
    CHECK(!strcmp(f[0].name,"code")&&f[0].offset==offsetof(struct mps_record,code)&&f[0].native_size==sizeof a.code&&f[0].width==2);
    CHECK(!strcmp(f[1].name,"value")&&f[1].offset==offsetof(struct mps_record,value)&&f[1].native_size==sizeof a.value&&f[1].width==4);
    CHECK(!strcmp(f[2].name,"note")&&f[2].offset==offsetof(struct mps_record,note)&&f[2].native_size==sizeof a.note&&f[2].width==5&&!f[2].required&&f[2].since==2);
    CHECK(mps_validate(&a)==MPS_OK&&mps_equal(&a,&b,&equal)==MPS_OK&&equal==1);b.note=-4;CHECK(mps_equal(&a,&b,&equal)==MPS_OK&&!equal);
    a.present=b.present=3;CHECK(mps_equal(&a,&b,&equal)==MPS_OK&&equal);a.present=1;equal=99;CHECK(mps_validate(&a)==MPS_INVALID&&mps_equal(&a,&b,&equal)==MPS_INVALID&&equal==99);
    a.present=11;CHECK(mps_validate(&a)==MPS_INVALID);CHECK(mps_validate(NULL)==MPS_INVALID);puts("MPS_SEMANTICS registry-offsets present-only equality required-mask");return 0;
}
static int visits(void){
    struct mps_record a={7,1,2,-3};struct mp_visits v={0,2,0,0};size_t n=99;CHECK(mps_visit(&a,visit,&v,&n)==MPS_STOP&&n==2&&v.calls==2&&v.tags==12&&v.sum==3);
    v=(struct mp_visits){0,0,0,0};CHECK(mps_visit(&a,visit,&v,&n)==MPS_OK&&n==3&&v.calls==3&&v.tags==123&&v.sum==0);
    a.present=1;n=99;v.calls=0;CHECK(mps_visit(&a,visit,&v,&n)==MPS_INVALID&&n==99&&v.calls==0);
    CHECK(mps_visit(&a,NULL,&v,&n)==MPS_INVALID);puts("MPS_VISIT validate-before-effects ordered-once stop-count");return 0;
}
static int base(void){
    const struct mp_field *f=NULL;unsigned char bytes[6]={0},expected[6]={0x12,0x34,0x89,0xab,0xcd,0xef};CHECK(mp_fields(&f)==2&&f&&!strcmp(f[1].name,"value")&&f[1].offset==offsetof(struct mp_record,value));CHECK(mp_record_encode((struct mp_record){0x1234,0x89abcdef},bytes,6)&&!memcmp(bytes,expected,6));puts("MPS_BASE retained-registry six-independent-wire-octets");return 0;
}
static int oracle(void){
    char op;while(scanf(" %c",&op)==1){
        if(op=='E'){unsigned version,present,code;uint32_t value;int32_t note;size_t cap,n=999,i;unsigned char bytes[64];int status;if(scanf("%u %u %u %" SCNu32 " %" SCNd32 " %zu",&version,&present,&code,&value,&note,&cap)!=6)return 2;memset(bytes,0xa5,64);struct mps_record r={(uint32_t)present,(uint16_t)code,value,note};status=mps_encode(version,&r,bytes,cap,&n);printf("%d %zu ",status,n);for(i=0;i<64;++i)printf("%02x",bytes[i]);puts("");}
        else if(op=='D'){char hex[131];unsigned char bytes[65];size_t i,n;struct mps_record r={3,99,88,77};int status;if(scanf("%130s",hex)!=1)return 2;n=!strcmp(hex,"-")?0:strlen(hex)/2;for(i=0;i<n;++i){unsigned u;if(sscanf(hex+2*i,"%2x",&u)!=1)return 2;bytes[i]=(unsigned char)u;}status=mps_decode(bytes,n,&r);printf("%d %" PRIu32 " %u %" PRIu32 " %" PRId32 "\n",status,r.present,(unsigned)r.code,r.value,r.note);}
        else if(op=='V'){unsigned present,code,stop;uint32_t value;int32_t note;size_t n=999;struct mp_visits v={0,0,0,0};if(scanf("%u %u %" SCNu32 " %" SCNd32 " %u",&present,&code,&value,&note,&stop)!=5)return 2;struct mps_record r={(uint32_t)present,(uint16_t)code,value,note};v.stop=stop;int status=mps_visit(&r,visit,&v,&n);printf("%d %zu %u %u %" PRId64 "\n",status,n,v.calls,v.tags,v.sum);}
        else return 2;
    }return 0;
}
static int metrics(void){
    unsigned sample,i;double seconds[7];uint64_t checksum=0;struct mps_record r={7,17,123456,-9};unsigned char bytes[64];size_t n=0;
    for(sample=0;sample<7;++sample){clock_t start=clock();for(i=0;i<10000;++i){CHECK(mps_encode(2,&r,bytes,64,&n)==MPS_OK);checksum+=n;}seconds[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;}
    printf("{\"iterations\":10000,\"checksum\":%" PRIu64 ",\"record_size\":%zu,\"descriptor_size\":%zu,\"wire_bytes\":%zu,\"seconds\":[",checksum,sizeof r,sizeof(struct mps_field),n);
    for(i=0;i<7;++i)printf("%s%.9f",i?",":"",seconds[i]);
    puts("]}");return 0;
}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"schema_wire"))return wire();if(!strcmp(argv[1],"schema_semantics"))return semantics();if(!strcmp(argv[1],"schema_visit"))return visits();if(!strcmp(argv[1],"schema_base"))return base();if(!strcmp(argv[1],"oracle"))return oracle();if(!strcmp(argv[1],"metrics"))return metrics();return 2;}
