#ifndef CMASTERY_MPT_LAW_TRACES_H
#define CMASTERY_MPT_LAW_TRACES_H
#include <stddef.h>
#include <stdint.h>
enum { MPT_OK,MPT_INVALID,MPT_NOMEM,MPT_RANGE,MPT_NO_FAILURE };
enum { MPT_BYTES,MPT_ASCII_TEXT };
typedef struct {const unsigned char *bytes;size_t length;unsigned cache;} MptView;
typedef void *(*MptAllocate)(void *,size_t);
typedef void (*MptFree)(void *,void *);
typedef struct {MptAllocate allocate;MptFree release;void *context;} MptAllocator;
typedef struct {unsigned char *bytes;size_t length;unsigned cache;MptAllocator allocator;} MptOwner;
int mpt_assign(MptOwner *,MptView,MptAllocator);
void mpt_drop(MptOwner *);
MptView mpt_view(const MptOwner *);
int mpt_equal(MptView,MptView,unsigned,unsigned *);
int mpt_hash(MptView,unsigned,uint64_t *);
#endif
