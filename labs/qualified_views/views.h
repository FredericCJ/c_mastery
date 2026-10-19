#ifndef FV_VIEWS_H
#define FV_VIEWS_H
#include <stddef.h>
typedef enum { FV_OK,FV_INVALID,FV_FULL,FV_RANGE,FV_UNFINISHED } FvStatus;
typedef struct { int values[4]; int *link; } FvRecord;
typedef struct { const int *data; size_t length; } FvView;
typedef struct { int *data; size_t capacity; } FvDestination;
FvStatus fv_reset(FvRecord *owner);
FvView fv_initialize(FvRecord *caller_owner);
FvStatus fv_update(FvRecord *owner,size_t index,int value);
FvView fv_view(const FvRecord *owner);
/* Nonempty source/destination accessed ranges must be disjoint. */
FvStatus fv_copy(FvDestination destination,FvView source);
/* All positions refer to the same admitted array; overlap is permitted. */
FvStatus fv_move(int *base,size_t extent,size_t destination,size_t source,size_t count);
typedef struct FvOwner FvOwner;
FvOwner *fv_owner(void);
FvView fv_owner_view(const FvOwner *owner);
int *fv_owner_mutable(FvOwner *owner);
/* Ordinary RAM device model only; no hardware or interthread guarantee. */
unsigned fv_device_twice(volatile unsigned *status);
#endif
