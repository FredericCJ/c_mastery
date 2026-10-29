#include "buffer.h"
static int allocator_valid(struct ob_allocator a){return a.allocate && a.resize && a.release;}
int ob_construct(struct ob_construction *c,int number,void *pointer,char tag,unsigned fail_after)
{
    if(!c)return -1;
    c->initialized=0;c->value.number=number;c->initialized=1;if(fail_after==1)return -1;
    c->value.pointer=pointer;c->initialized|=2;if(fail_after==2)return -1;
    c->value.tag=tag;c->initialized|=4;return 0;
}
int ob_record_equal(const struct ob_record *a,const struct ob_record *b)
{return a && b && a->number==b->number && a->pointer==b->pointer && a->tag==b->tag;}
int ob_record_encode(const struct ob_record *r,unsigned char *bytes,size_t n)
{
    if(!r || !bytes || n<4 || r->number<0 || r->number>65535)return -1;
    bytes[0]=(unsigned char)((unsigned)r->number>>8);bytes[1]=(unsigned char)r->number;bytes[2]=(unsigned char)r->tag;bytes[3]=(unsigned char)(r->pointer!=NULL);return 0;
}
int ob_cleanup(struct ob_construction *c,struct ob_allocator a)
{
    if(!c || !a.release)return -1;
#ifdef OB_MUTANT_CLEANUP
    if(c->initialized&1u)a.release(a.context,NULL); /* False release event, without reading uninitialized pointer. */
#else
    if((c->initialized&2u) && c->value.pointer)a.release(a.context,c->value.pointer);
#endif
    c->initialized=0;return 0;
}
int ob_size(size_t prefix,size_t width,size_t count,size_t limit,size_t *out)
{
    if(!out || !width || prefix>limit || count>(limit-prefix)/width)return -1;
#ifdef OB_MUTANT_SIZE
    *out=prefix;
#else
    *out=prefix+count*width;
#endif
    return 0;
}
int ob_create(struct ob_table *t,struct ob_allocator a,uint32_t identity,size_t capacity)
{
    size_t bytes;struct ob_flex *p;
    if(!t || t->value || t->retired || !identity || !allocator_valid(a) || ob_size(sizeof(struct ob_flex),1,capacity,SIZE_MAX,&bytes))return -1;
    p=a.allocate(a.context,bytes);if(!p)return -1;p->length=0;p->capacity=capacity;
    for(size_t i=0;i<capacity;++i)p->payload[i]=0;
    t->value=p;t->allocation=a;t->identity=identity;t->generation=1;return 0;
}
int ob_resize(struct ob_table *t,size_t capacity)
{
    size_t bytes,oldbytes,oldcapacity;struct ob_flex *p;
    if(!t || !t->value || !capacity || t->retired || t->generation>=3 || ob_size(sizeof(struct ob_flex),1,capacity,SIZE_MAX,&bytes))return -1;
    oldcapacity=t->value->capacity;oldbytes=sizeof(struct ob_flex)+oldcapacity;
    p=t->allocation.resize(t->allocation.context,t->value,oldbytes,bytes);
#ifdef OB_MUTANT_RESIZE
    if(!p){++t->generation;return -1;}
#else
    if(!p)return -1;
#endif
    /* Never evaluate the old pointer after successful resize. */
    t->value=p;++t->generation;if(p->length>capacity)p->length=capacity;p->capacity=capacity;
    for(size_t i=oldcapacity;i<capacity;++i)p->payload[i]=0;
    return 0;
}
int ob_append(struct ob_table *t,unsigned char byte)
{if(!t || !t->value || t->retired || t->value->length==t->value->capacity)return -1;t->value->payload[t->value->length++]=byte;return 0;}
int ob_erase(struct ob_table *t,size_t index)
{
    if(!t || !t->value || t->retired || index>=t->value->length || t->generation>=3)return -1;
    for(size_t i=index;i+1<t->value->length;++i)t->value->payload[i]=t->value->payload[i+1];
    --t->value->length;
#ifndef OB_MUTANT_ERASE
    ++t->generation;
#endif
    return 0;
}
void ob_reset(struct ob_table *t)
{
    if(!t)return;
    if(t->value)t->allocation.release(t->allocation.context,t->value);
    t->value=NULL;t->retired=1; /* Identity is permanently retired, even with generation headroom. */
}
int ob_handle(const struct ob_table *t,size_t index,struct ob_handle *out)
{if(!t || !out || !t->value || t->retired || index>=t->value->length)return -1;*out=(struct ob_handle){t->identity,t->generation,index};return 0;}
int ob_resolve(const struct ob_table *t,struct ob_handle h,unsigned char *out)
{
    if(!t || !out || !t->value || t->retired || h.identity!=t->identity || h.index>=t->value->length)return -1;
#ifndef OB_MUTANT_GENERATION
    if(h.generation!=t->generation)return -1;
#endif
    *out=t->value->payload[h.index];return 0;
}
int ob_table_move(struct ob_table *to,struct ob_table *from)
{
    if(!to || !from || to==from || to->value || to->retired || !from->value || from->retired)return -1;
    *to=*from;from->value=NULL;from->identity=0;from->retired=1;return 0;
}
int ob_split_create(struct ob_allocator a,size_t capacity,struct ob_split **out)
{
    struct ob_split *p;
    if(!out || !allocator_valid(a))return -1;
    p=a.allocate(a.context,sizeof *p);if(!p)return -1;
    p->length=0;p->capacity=capacity;p->payload=NULL;p->allocation=a;
    if(capacity){p->payload=a.allocate(a.context,capacity);if(!p->payload){a.release(a.context,p);return -1;}for(size_t i=0;i<capacity;++i)p->payload[i]=0;}
    *out=p;return 0;
}
void ob_split_drop(struct ob_split **owner)
{if(owner && *owner){struct ob_split *p=*owner;struct ob_allocator a=p->allocation;if(p->payload)a.release(a.context,p->payload);a.release(a.context,p);*owner=NULL;}}
int ob_shared_create(struct ob_allocator a,const unsigned char *bytes,size_t n,struct ob_shared **out)
{
    size_t size;struct ob_shared *p;
    if(!out || (!bytes && n) || !allocator_valid(a) || ob_size(sizeof(struct ob_shared),1,n,SIZE_MAX,&size))return -1;
    p=a.allocate(a.context,size);if(!p)return -1;p->references=1;p->length=n;p->allocation=a;
    for(size_t i=0;i<n;++i)p->payload[i]=bytes[i];
    *out=p;return 0;
}
int ob_shared_retain(struct ob_shared *p,struct ob_shared **out)
{
    if(!p || !out || !p->references || p->references>=3)return -1;
#ifndef OB_MUTANT_SHARED
    ++p->references;
#endif
    *out=p;return 0;
}
void ob_shared_drop(struct ob_shared **owner)
{if(owner && *owner){struct ob_shared *p=*owner;*owner=NULL;if(--p->references==0){struct ob_allocator a=p->allocation;a.release(a.context,p);}}}
