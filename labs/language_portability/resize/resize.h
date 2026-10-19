#ifndef LP_RESIZE_H
#define LP_RESIZE_H
#include <stddef.h>
typedef enum {LR_OK,LR_INVALID,LR_NOMEM} LrStatus;
typedef struct {
    void *context;
    /* Nonzero operation: preserve old allocation on NULL; success transfers
       min(old,new) bytes to returned owned storage and invalidates old borrows. */
    void *(*resize)(void *,void *,size_t,size_t);
    void (*release)(void *,void *,size_t);
} LrAllocator;
typedef struct {unsigned char *data;size_t size;LrAllocator allocator;} LrBuffer;
/* Init only fresh/nonowning control; live noncopied controls are serialized.
   Callbacks are nonreentrant, do not mutate control, and retain no borrowed
   pointer. Resize admits1..64 only; zero is refused before any callback. */
LrStatus lr_init(LrBuffer *,LrAllocator);
LrStatus lr_resize(LrBuffer *,size_t);
void lr_release(LrBuffer *);
#endif
