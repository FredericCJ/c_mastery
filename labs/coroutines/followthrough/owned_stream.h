#ifndef CMASTERY_OWNED_STREAM_H
#define CMASTERY_OWNED_STREAM_H
#include "coroutines.h"
struct cf_item { unsigned char *data;size_t length;struct co_allocator allocator; };
typedef int (*cf_map_fn)(void *,uint32_t,struct cf_item *);
struct cf_source { const uint32_t *values;size_t length,index; };
int cf_item_drop(struct cf_item *);
int cf_map_next(struct cf_source *,cf_map_fn,void *,struct cf_item *);
int cf_zip_next(struct cf_source *,struct cf_source *,cf_map_fn,void *,struct cf_item *,struct cf_item *);
struct cf_generator {
    struct cf_generator *self;
    struct co_allocator allocator;
    uint32_t a,b;
    unsigned index,count;
    struct cf_item current;
};
int cf_gen_create(struct co_allocator,unsigned,struct cf_generator **);
int cf_gen_next(struct cf_generator *);
int cf_gen_borrow(struct cf_generator *,const unsigned char **,size_t *);
int cf_gen_take(struct cf_generator *,struct cf_item *);
int cf_gen_clone(struct cf_generator *,struct cf_item *);
int cf_gen_destroy(struct cf_generator **);
#endif
