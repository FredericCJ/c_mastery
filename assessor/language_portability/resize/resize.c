#include "resize.h"
#ifndef LR_MUTANT
#define LR_MUTANT 0
#endif
enum lr_status lr_init(struct lr_buffer *b,struct lr_allocator a){
    if(!b || !a.resize || !a.release)return LR_INVALID;
    b->data=NULL;b->size=0;b->allocator=a;return LR_OK;
}
enum lr_status lr_resize(struct lr_buffer *b,size_t size){
    if(!b || !b->allocator.resize || !b->allocator.release)return LR_INVALID;
#if LR_MUTANT == 1
    if(!size)(void)b->allocator.resize(b->allocator.context,b->data,b->size,0);
#endif
    if(!size || size>64)return LR_INVALID;
    unsigned char *next=b->allocator.resize(b->allocator.context,b->data,b->size,size);
#if LR_MUTANT == 2
    b->data=next;
#endif
    if(!next)return LR_NOMEM;
#if LR_MUTANT != 4
    for(size_t i=b->size;i<size;i++)next[i]=0;
#endif
    b->data=next;b->size=size;return LR_OK;
}
void lr_release(struct lr_buffer *b){
    if(!b || !b->data)return;
    unsigned char *old=b->data;size_t size=b->size;struct lr_allocator a=b->allocator;
    b->data=NULL;b->size=0;
#if LR_MUTANT == 3
    (void)old;(void)size;(void)a;
#else
    a.release(a.context,old,size);
#endif
}
