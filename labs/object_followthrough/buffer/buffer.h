#ifndef OB_BUFFER_H
#define OB_BUFFER_H
#include <stddef.h>
#include <stdint.h>
typedef struct { void *context; void *(*allocate)(void *,size_t);void *(*resize)(void *,void *,size_t,size_t);void (*release)(void *,void *); } ObAllocator;
typedef struct { int number;void *pointer;char tag; } ObRecord;
typedef struct { ObRecord value;unsigned initialized; } ObConstruction;
int ob_construct(ObConstruction *,int,void *,char,unsigned);
int ob_record_equal(const ObRecord *,const ObRecord *);
int ob_record_encode(const ObRecord *,unsigned char *,size_t);
int ob_cleanup(ObConstruction *,ObAllocator);
int ob_size(size_t,size_t,size_t,size_t,size_t *);
typedef struct { size_t length,capacity;unsigned char payload[]; } ObFlex;
typedef struct { double alignment;unsigned char tag;unsigned char payload[]; } ObPadded;
typedef struct { ObFlex *value;ObAllocator allocation;uint32_t identity,generation;int retired; } ObTable;
typedef struct { uint32_t identity,generation;size_t index; } ObHandle;
int ob_create(ObTable *,ObAllocator,uint32_t,size_t);
int ob_resize(ObTable *,size_t);
int ob_append(ObTable *,unsigned char);
int ob_erase(ObTable *,size_t);
void ob_reset(ObTable *);
int ob_handle(const ObTable *,size_t,ObHandle *);
int ob_resolve(const ObTable *,ObHandle,unsigned char *);
int ob_table_move(ObTable *,ObTable *);
typedef struct {size_t length,capacity;unsigned char *payload;ObAllocator allocation;} ObSplit;
int ob_split_create(ObAllocator,size_t,ObSplit **);
void ob_split_drop(ObSplit **);
typedef struct {size_t references,length;ObAllocator allocation;unsigned char payload[];} ObShared;
int ob_shared_create(ObAllocator,const unsigned char *,size_t,ObShared **);
int ob_shared_retain(ObShared *,ObShared **);
void ob_shared_drop(ObShared **);
int ob_workspace(int,int *,size_t,int *,unsigned *);
#endif
