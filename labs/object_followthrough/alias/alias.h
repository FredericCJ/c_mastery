#ifndef OS_ALIAS_H
#define OS_ALIAS_H
#include "depth.h"
typedef struct { FDAllocator allocator; size_t capacity,alignment; } OsAllocator;
typedef struct { _Alignas(64) int value; unsigned char *resource; } OsWide;
typedef int (*OsDrop)(void *,OsWide *);
typedef enum { OS_EMPTY,OS_LIVE,OS_BORROWED,OS_CLEANUP } OsState;
typedef struct { OsWide *value; OsAllocator allocation; OsDrop drop; void *context; OsState state; } OsWideSlot;
int os_acquire(FDSlot *,OsAllocator);
int os_wide_acquire(OsWideSlot *,OsAllocator,int,OsDrop,void *);
int os_wide_borrow(OsWideSlot *,const OsWide **);
int os_wide_return(OsWideSlot *,const OsWide **);
int os_wide_reset(OsWideSlot *);
int os_clone(void *,const FDValue *);
int os_wire(const unsigned char *,size_t,FDValue *);
uint32_t os_host_representation(uint32_t);
uint32_t os_wire_integer(const unsigned char *);
void os_transform(unsigned char *restrict,const unsigned char *restrict,size_t,unsigned char);
void os_ordinary(unsigned char *,const unsigned char *,size_t,unsigned char);
void os_nested(unsigned char *restrict,const unsigned char *restrict,size_t,unsigned char);
void os_shared(unsigned char *restrict,const unsigned char *restrict,const unsigned char *restrict,size_t);
int os_partition(unsigned char *,size_t,size_t,size_t,size_t,unsigned char,int);
FDStatus os_move(unsigned char *,size_t,size_t,size_t,size_t);
typedef struct { uint32_t prefix; int member; } OsContainer;
int os_member(OsContainer *,int *,int *);
#ifdef UINTPTR_MAX
int os_tag_roundtrip(OsContainer *,unsigned,uintptr_t *,int *);
#endif
#endif
