#include "evolution_record.h"
#include "hashing.h"
#include "foundation.h"
#ifndef ADE_MUTATION
#define ADE_MUTATION 0
#endif
static int valid(const struct ade_record *r){
    unsigned seen=0;if(!r||r->count>ADE_FIELDS)return 0;
    for(unsigned i=0;i<r->count;i++){
        const struct ade_field *f=&r->fields[i];unsigned char wire[ADE_PAYLOAD+2];size_t used=0;
        if(f->length>ADE_PAYLOAD||!ad_evolve_write((struct ad_evolving){f->tag,f->bytes,f->length},wire,sizeof wire,&used))return 0;
        if(f->tag<=2){unsigned bit=1u<<f->tag;if((seen&bit)&&ADE_MUTATION!=1)return 0;seen|=bit;}
    }return 1;
}
int ade_read(const unsigned char *wire,size_t n,struct ade_record *out){
    if(!wire||!out||!n||n>ADE_BYTES||wire[0]>ADE_FIELDS)return ADE_INVALID;
    struct ade_record next={0};next.count=wire[0];struct f_cursor cursor={wire,n,0};const uint8_t *header,*payload;
    if(f_take(&cursor,1,&header)!=F_OK)return ADE_INVALID;
    for(unsigned i=0;i<next.count;i++){
        if(f_take(&cursor,2,&header)!=F_OK)return ADE_INVALID;
        size_t length=header[1];if(length>ADE_PAYLOAD||f_take(&cursor,length,&payload)!=F_OK)return ADE_INVALID;
        struct ad_evolving view;if(!ad_evolve_read(header,length+2,&view))return ADE_INVALID;
        next.fields[i].tag=view.tag;next.fields[i].length=(unsigned)view.length;
        for(size_t j=0;j<view.length;j++)next.fields[i].bytes[j]=view.payload[j];
        (void)payload;
    }
    if(cursor.position!=n||!valid(&next))return ADE_INVALID;
    *out=next;return ADE_OK;
}
int ade_write(const struct ade_record *r,int canonical,unsigned char *out,size_t capacity,size_t *used){
    if(!out||!used||(canonical!=0&&canonical!=1)||!valid(r))return ADE_INVALID;
    size_t total=1;unsigned order[ADE_FIELDS];
    for(unsigned i=0;i<r->count;i++){order[i]=i;total+=2+r->fields[i].length;}
    if(capacity<total)return ADE_CAPACITY;
    if(canonical)for(unsigned i=1;i<r->count;i++){
        unsigned index=order[i],j=i;
        while(j&&(ADE_MUTATION==2?r->fields[order[j-1]].tag>=r->fields[index].tag:r->fields[order[j-1]].tag>r->fields[index].tag)){order[j]=order[j-1];j--;}
        order[j]=index;
    }
    unsigned char staged[ADE_BYTES];staged[0]=(unsigned char)r->count;size_t at=1;
    for(unsigned i=0;i<r->count;i++){
        const struct ade_field *f=&r->fields[order[i]];size_t count=0;
        if(!ad_evolve_write((struct ad_evolving){f->tag,f->bytes,f->length},staged+at,sizeof staged-at,&count))return ADE_INVALID;
        at+=count;
    }
    for(size_t i=0;i<at;i++)out[i]=staged[i];
    *used=at;return ADE_OK;
}
int ade_tag(const struct ade_record *r,const unsigned char key[32],unsigned char tag[32]){
    if(!key||!tag||!valid(r))return ADE_INVALID;
    static const unsigned char domain[]="CMASTERY-ADT-RECORD-v1";
    unsigned char transcript[sizeof domain+ADE_BYTES],digest[32];size_t used=0;
    for(size_t i=0;i<sizeof domain;i++)transcript[i]=domain[i];
    struct ade_record projected=*r;
    if(ADE_MUTATION==3){projected.count=0;for(unsigned i=0;i<r->count;i++)if(r->fields[i].tag<=2)projected.fields[projected.count++]=r->fields[i];}
    int status=ade_write(&projected,1,transcript+sizeof domain,ADE_BYTES,&used);
    if(status!=ADE_OK)return status;
    if(rph_hmac(key,32,transcript,sizeof domain+used,digest)!=RPH_OK)return ADE_INVALID;
    for(unsigned i=0;i<32;i++)tag[i]=digest[i];
    rph_clear(digest,sizeof digest);return ADE_OK;
}
int ade_check(const struct ade_record *r,const unsigned char key[32],const unsigned char tag[32]){
    if(!tag)return ADE_INVALID;
    unsigned char expected[32];int status=ade_tag(r,key,expected);if(status!=ADE_OK)return status;
    int same=ADE_MUTATION==4?expected[0]==tag[0]:rph_equal(expected,tag);rph_clear(expected,sizeof expected);return same?ADE_OK:ADE_AUTH;
}
int ade_apply(const struct ade_record *r,const unsigned char key[32],const unsigned char tag[32],struct ade_service *service){
    if(!service)return ADE_INVALID;
    int status=ade_check(r,key,tag);if(status!=ADE_OK)return status;
    uint32_t amount=0;unsigned found=0;
    for(unsigned i=0;i<r->count;i++){
        if(r->fields[i].tag>2){if(ADE_MUTATION!=5)return ADE_POLICY;continue;}
        if(r->fields[i].tag==1){amount=(uint32_t)r->fields[i].bytes[0]*256+r->fields[i].bytes[1];found=1;}
    }
    if(!found)return ADE_POLICY;
    if(service->total>UINT32_MAX-amount){if(ADE_MUTATION==6)service->privileged++;return ADE_RANGE;}
    service->total+=amount;return ADE_OK;
}
