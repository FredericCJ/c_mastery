#include "schema.h"
#include "arch.h"
#ifndef MPS_MUTATION
#define MPS_MUTATION 0
#endif
_Static_assert(MPS_FIELD_COUNT<=8,"bounded schema");
#define MPS_DESCRIPTOR(i,name,type,tag,width,required,kind,lo,hi,since) {#name,offsetof(struct mps_record,name),sizeof(type),i,tag,width,required,MPS_##kind,since,lo,hi},
static const struct mps_field fields[]={MPS_FIELDS(MPS_DESCRIPTOR)};
#undef MPS_DESCRIPTOR
void MPS_LINK_GUARD(void) {}
size_t mps_fields(const struct mps_field **out){if(!out)return 0;*out=fields;return MPS_FIELD_COUNT;}
static int64_t get(const struct mps_record *r,unsigned index){
    switch(index){
#define MPS_GET(i,name,type,tag,width,required,kind,lo,hi,since) case i:return r->name;
    MPS_FIELDS(MPS_GET)
#undef MPS_GET
    default:return 0;
    }
}
static void set(struct mps_record *r,unsigned index,int64_t value){
    switch(index){
#define MPS_SET(i,name,type,tag,width,required,kind,lo,hi,since) case i:r->name=(type)value;break;
    MPS_FIELDS(MPS_SET)
#undef MPS_SET
    default:break;
    }
}
int mps_validate(const struct mps_record *r){
    unsigned i;uint32_t known=0;
    if(!r)return MPS_INVALID;
    for(i=0;i<MPS_FIELD_COUNT;++i){const struct mps_field *f=&fields[i];uint32_t bit=UINT32_C(1)<<f->index;known|=bit;
        if(f->required && !(r->present&bit) && MPS_MUTATION!=6)return MPS_INVALID;
        if(r->present&bit){int64_t v=get(r,f->index);if(v<f->minimum||v>f->maximum)return MPS_RANGE;}
    }
    return r->present&~known?MPS_INVALID:MPS_OK;
}
int mps_equal(const struct mps_record *a,const struct mps_record *b,unsigned *out){
    unsigned i,equal=1;if(!out)return MPS_INVALID;
    if(mps_validate(a)!=MPS_OK||mps_validate(b)!=MPS_OK)return MPS_INVALID;
    if(a->present!=b->present)equal=0;
    for(i=0;i<MPS_FIELD_COUNT;++i){const struct mps_field *f=&fields[i];if(MPS_MUTATION==1&&!f->required)continue;if((a->present&(UINT32_C(1)<<f->index)) && (b->present&(UINT32_C(1)<<f->index)) && get(a,f->index)!=get(b,f->index))equal=0;}
    *out=equal;return MPS_OK;
}
int mps_encode(unsigned version,const struct mps_record *r,unsigned char *out,size_t capacity,size_t *used){
    unsigned char temporary[MPS_WIRE_CAPACITY];size_t n=2;unsigned i,count=0;int valid;
    if(!out||!used||!version||version>MPS_VERSION)return MPS_INVALID;
    valid=mps_validate(r);if(valid!=MPS_OK)return valid;
    temporary[0]=(unsigned char)version;
    for(i=0;i<MPS_FIELD_COUNT;++i){const struct mps_field *f=&fields[i];int64_t v;
        if(!(r->present&(UINT32_C(1)<<f->index)))continue;
        if(f->since>version)return MPS_RANGE;
        if(f->width+2>MPS_WIRE_CAPACITY-n)return MPS_RANGE;
        temporary[n++]=(unsigned char)f->tag;temporary[n++]=(unsigned char)f->width;v=get(r,f->index);
        if(f->kind==MPS_S32){if(ar_encode(2,(struct ar_record){(int32_t)v},temporary+n,f->width)!=AR_OK)return MPS_INVALID;}
        else{unsigned j;uint32_t u=(uint32_t)v;for(j=0;j<f->width;++j)temporary[n+j]=(unsigned char)(u>>((MPS_MUTATION==4?j:f->width-1-j)*8));}
        n+=f->width;++count;
    }
    temporary[1]=(unsigned char)count;if(n>capacity)return MPS_FULL;
    for(i=0;i<n;++i)out[i]=temporary[i];
    *used=n;return MPS_OK;
}
int mps_decode(const unsigned char *wire,size_t length,struct mps_record *out){
    struct mps_record staged={0};size_t at=2;unsigned count,version,last=0,i;
    if(!wire||!out||length<2||length>MPS_WIRE_CAPACITY)return MPS_INVALID;
    version=wire[0];count=wire[1];if(!version||version>MPS_VERSION||count>MPS_FIELD_COUNT)return MPS_INVALID;
    for(i=0;i<count;++i){unsigned tag,width,j;const struct mps_field *f=NULL;int64_t value;
        if(length-at<2)return MPS_INVALID;
        tag=wire[at++];width=wire[at++];if(tag<=last)return MPS_INVALID;last=tag;
        for(j=0;j<MPS_FIELD_COUNT;++j)if(fields[j].tag==tag){f=&fields[j];break;}
        if(!f||f->since>version||width!=f->width||width>length-at)return MPS_INVALID;
        if(f->kind==MPS_S32){struct ar_record decoded;unsigned char payload[5];for(j=0;j<5;++j)payload[j]=wire[at+j];if(MPS_MUTATION==5)payload[4]=(unsigned char)(payload[0]^payload[1]^payload[2]^payload[3]);if(ar_decode(2,payload,5,&decoded)!=AR_OK)return MPS_INVALID;value=decoded.value;}
        else{uint32_t u=0;for(j=0;j<width;++j)u=(u<<8)|wire[at+j];value=u;}
        if(value<f->minimum||value>f->maximum)return MPS_RANGE;
        set(&staged,f->index,value);staged.present|=UINT32_C(1)<<f->index;at+=width;
        if(MPS_MUTATION==2)*out=staged;
    }
    if(at!=length||mps_validate(&staged)!=MPS_OK)return MPS_INVALID;
    *out=staged;return MPS_OK;
}
int mps_visit(const struct mps_record *r,mps_visitor_fn visit,void *context,size_t *visited){
    unsigned i;size_t count=0;int valid;if(!visit||!visited)return MPS_INVALID;valid=mps_validate(r);if(valid!=MPS_OK)return valid;
    for(i=0;i<MPS_FIELD_COUNT;++i){const struct mps_field *f=&fields[i];if(!(r->present&(UINT32_C(1)<<f->index)))continue;++count;if(!visit(context,f,get(r,f->index))&&MPS_MUTATION!=3){*visited=count;return MPS_STOP;}}
    *visited=count;return MPS_OK;
}
