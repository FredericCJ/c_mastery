#include "atlas.h"
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>
int oa_resize(struct oa_owner *owner,size_t n,const struct oa_allocator *a)
{
    unsigned char *next;
    size_t i,copy;
    if(!owner || !a || !a->get || !a->put || !n || owner->generation==UINT_MAX || (!owner->data && owner->length)) return -1;
    next=a->get(a->context,n);
    if(!next) {
#ifdef OA_MUTANT_FAIL
        owner->length=0;
#endif
        return -1;
    }
    copy=owner->length<n?owner->length:n;
    for(i=0;i<copy;++i)next[i]=owner->data[i];
    for(;i<n;++i)next[i]=0;
    if(owner->data)a->put(a->context,owner->data);
    owner->data=next;owner->length=n;++owner->generation;return 0;
}
void oa_release(struct oa_owner *owner,const struct oa_allocator *a)
{
    if(!owner || !a || !a->put)return;
    if(owner->data)a->put(a->context,owner->data);
    owner->data=NULL;owner->length=0;
}
int oa_deep_copy(struct oa_view source,char *dest,size_t cap,struct oa_view *out)
{
    if(!out || !dest || !source.data || source.length>cap)return -1;
#ifdef OA_MUTANT_BORROW
    *out=source;return 0;
#else
    for(size_t i=0;i<source.length;++i)dest[i]=source.data[i];
    out->data=dest;out->length=source.length;return 0;
#endif
}
int oa_label(char *dest,size_t cap,struct oa_view *out)
{
    const char text[4]={'c','a','t','\0'};
    if(!out || !dest || cap<4)return -1;
    for(size_t i=0;i<4;++i)dest[i]=text[i];
    out->data=dest;out->length=3;return 0;
}
int oa_equal(const struct oa_record *a,const struct oa_record *b)
{
    if(!a || !b)return 0;
#ifdef OA_MUTANT_BYTES
    return memcmp(a,b,sizeof *a)==0;
#else
    return a->marker==b->marker && a->number==b->number &&
           (a->measure==b->measure || (isnan(a->measure)&&isnan(b->measure)));
#endif
}
int oa_encode(const struct oa_record *record,unsigned char *out,size_t cap)
{
    unsigned char encoded[13];uint64_t bits,onebits;double one=1.0;uint32_t number;
    if(!record || !out || cap!=13)return -1;
    if(CHAR_BIT!=8 || sizeof(double)!=8 || FLT_RADIX!=2 || DBL_MANT_DIG!=53 || DBL_MAX_EXP!=1024)return -2;
    memcpy(&onebits,&one,sizeof onebits);
    if(onebits!=UINT64_C(0x3ff0000000000000))return -2;
    memcpy(&bits,&record->measure,sizeof bits);
    if(isnan(record->measure))bits=UINT64_C(0x7ff8000000000000);
#ifndef OA_MUTANT_ZERO
    else if(record->measure==0.0)bits=0;
#endif
    number=(uint32_t)record->number;
    encoded[0]=(unsigned char)record->marker;
    for(size_t i=0;i<4;++i)encoded[1+i]=(unsigned char)(number>>(24-8*i));
    for(size_t i=0;i<8;++i)encoded[5+i]=(unsigned char)(bits>>(56-8*i));
    for(size_t i=0;i<13;++i)out[i]=encoded[i];
    return 0;
}
int oa_place(size_t total,size_t start,size_t length,size_t width,size_t alignment,size_t *out)
{
    size_t padding;
    if(!out || !alignment || (alignment&(alignment-1)) || start>total || length>total-start)return -1;
    padding=(alignment-start%alignment)%alignment;
#ifdef OA_MUTANT_SLICE
    padding=0;
#endif
    if(padding>length || width>length-padding)return -1;
    *out=start+padding;return 0;
}
int oa_slot(struct oa_storage *storage,size_t offset,size_t capacity,struct oa_record **out)
{
    if(!storage || !out || offset%sizeof(struct oa_record) || offset>sizeof(*storage)-sizeof(struct oa_record) || capacity<sizeof(struct oa_record) || capacity>sizeof(*storage)-offset)return -1;
    *out=&storage->records[offset/sizeof(struct oa_record)];return 0;
}
int oa_wide_admit(size_t guarantee)
{
#ifdef OA_MUTANT_WIDE
    return guarantee>=_Alignof(max_align_t);
#else
    return guarantee && !(guarantee&(guarantee-1)) && guarantee>=_Alignof(struct oa_wide);
#endif
}
int oa_decode_u32(const unsigned char *bytes,size_t length,uint32_t *out)
{
    uint32_t value=0;
    if(!bytes || !out || length<4 || CHAR_BIT!=8)return -1;
    for(size_t i=0;i<4;++i)value=(value<<8)|bytes[i];
    *out=value;return 0;
}
