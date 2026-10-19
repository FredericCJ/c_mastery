#ifndef CMASTERY_OWNED_STREAM_H
#define CMASTERY_OWNED_STREAM_H
#include "coroutines.h"
typedef struct { unsigned char *data;size_t length;COAllocator allocator; } CFItem;
typedef int (*CFMap)(void *,uint32_t,CFItem *);
typedef struct { const uint32_t *values;size_t length,index; } CFSource;
int cf_item_drop(CFItem *);
int cf_map_next(CFSource *,CFMap,void *,CFItem *);
int cf_zip_next(CFSource *,CFSource *,CFMap,void *,CFItem *,CFItem *);
typedef struct CFGenerator {
    struct CFGenerator *self;
    COAllocator allocator;
    uint32_t a,b;
    unsigned index,count;
    CFItem current;
} CFGenerator;
int cf_gen_create(COAllocator,unsigned,CFGenerator **);
int cf_gen_next(CFGenerator *);
int cf_gen_borrow(CFGenerator *,const unsigned char **,size_t *);
int cf_gen_take(CFGenerator *,CFItem *);
int cf_gen_clone(CFGenerator *,CFItem *);
int cf_gen_destroy(CFGenerator **);
#endif
