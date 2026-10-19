#include "owned_stream.h"
#ifndef CF_MUTANT
#define CF_MUTANT 0
#endif
int cf_item_drop(CFItem *x) {
    if(!x)return CO_INVALID;
    if(x->data){COAllocator a=x->allocator;void *p=x->data;*x=(CFItem){0};a.release(a.context,p);}
    return CO_OK;
}
static int source_valid(const CFSource*s){return s&&s->index<=s->length&&(!s->length||s->values);}
int cf_map_next(CFSource*s,CFMap map,void*ctx,CFItem*out){
    if(!source_valid(s)||!map||!out)return CO_INVALID;
    if(s->index==s->length)return CO_DONE;
    uint32_t v=s->values[s->index++];CFItem item={0};int r=map(ctx,v,&item);
    if(r){
        if(CF_MUTANT!=1)cf_item_drop(&item);
        return CO_IO;
    }
    *out=item;return CO_OK;
}
int cf_zip_next(CFSource*a,CFSource*b,CFMap map,void*ctx,CFItem*x,CFItem*y){
    if(!source_valid(a)||!source_valid(b)||a==b||!map||!x||!y||x==y)return CO_INVALID;
    if(a->index==a->length||b->index==b->length){
        if(CF_MUTANT==2&&a->index<a->length)a->index++;
        return CO_DONE;
    }
    uint32_t av=a->values[a->index++],bv=b->values[b->index++];CFItem left={0},right={0};
    int r=map(ctx,av,&left);if(!r)r=map(ctx,bv,&right);
    if(r){cf_item_drop(&left);cf_item_drop(&right);return CO_IO;}
    *x=left;*y=right;return CO_OK;
}
static int gen_valid(CFGenerator*g){return g&&g->self==g&&g->index<=g->count&&g->count<=47;}
int cf_gen_create(COAllocator a,unsigned count,CFGenerator**out){
    if(!out||!a.allocate||!a.release)return CO_INVALID;if(count>47)return CO_RANGE;
    CFGenerator*g=a.allocate(a.context,sizeof *g);if(!g)return CO_RANGE;
    *g=(CFGenerator){g,a,0,1,0,count,{0}};*out=g;return CO_OK;
}
int cf_gen_next(CFGenerator*g){
    if(!gen_valid(g))return CO_INVALID;if(g->index==g->count)return CO_DONE;
    unsigned char*p=g->allocator.allocate(g->allocator.context,4);if(!p)return CO_RANGE;
    uint32_t v=g->a;for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));
    if(CF_MUTANT!=3)cf_item_drop(&g->current);
    g->current=(CFItem){p,4,g->allocator};
    if(g->index+1<g->count){uint32_t next=g->a+g->b;g->a=g->b;g->b=next;}g->index++;return CO_OK;
}
int cf_gen_borrow(CFGenerator*g,const unsigned char**out,size_t*n){
    if(!gen_valid(g)||!out||!n)return CO_INVALID;if(!g->current.data)return CO_STATE;
    *out=g->current.data;*n=g->current.length;return CO_OK;
}
int cf_gen_take(CFGenerator*g,CFItem*out){
    if(!gen_valid(g)||!out)return CO_INVALID;if(!g->current.data)return CO_STATE;
    *out=g->current;if(CF_MUTANT!=4)g->current=(CFItem){0};return CO_OK;
}
int cf_gen_clone(CFGenerator*g,CFItem*out){
    if(!gen_valid(g)||!out)return CO_INVALID;if(!g->current.data)return CO_STATE;
    unsigned char*p=g->allocator.allocate(g->allocator.context,g->current.length);if(!p)return CO_RANGE;
    for(size_t i=0;i<g->current.length;i++)p[i]=g->current.data[i];
    *out=(CFItem){p,g->current.length,g->allocator};return CO_OK;
}
int cf_gen_destroy(CFGenerator**owner){
    if(!owner)return CO_INVALID;if(!*owner)return CO_OK;CFGenerator*g=*owner;
    if(!gen_valid(g))return CO_INVALID;COAllocator a=g->allocator;cf_item_drop(&g->current);
    *owner=NULL;a.release(a.context,g);return CO_OK;
}
