#ifndef ALGORITHM_WRAPPERS_H
#define ALGORITHM_WRAPPERS_H
#include <stddef.h>
#include <stdint.h>
#include "containers.h"
typedef struct { int32_t id,rank; } AWRecord;
typedef struct { unsigned epoch,descending; } AWContext;
typedef struct { AWRecord values[3]; const AWRecord *pointers[3]; } AWBank;
typedef int (*AWCompare)(void *,const AWRecord *,const AWRecord *,int *);
typedef struct { void *context; AWBank *(*acquire)(void *); void (*release)(void *,AWBank *); } AWBacking;
enum AWStatus { AW_OK,AW_INVALID,AW_FULL,AW_DUPLICATE,AW_MISSING,AW_BACKING,AW_COMPARE,AW_BUSY,AW_STALE,AW_MUTATED,AW_EXHAUSTED };
enum AWKind { AW_VALUE,AW_BORROW };
enum AWDispatch { AW_FINITE,AW_CALLBACK };
typedef struct AWOwner {
 CDTree ids;
 AWBank *bank;
 AWRecord expected[3];
 AWBacking backing;
 AWContext *context;
 AWCompare compare;
 void *compare_context;
 unsigned count,capacity,generation,limit,context_epoch,descending,kind,dispatch,alive,busy;
} AWOwner;
typedef struct { const AWOwner *owner; unsigned generation,index; } AWView;
/* The kernel requires sorted actual Record objects, count<=3, direction0/1.
 * It returns the first position not less than query in rank/ID order. */
unsigned aw_lower(const AWRecord *,unsigned,const AWRecord *,unsigned);
unsigned aw_probe(const AWRecord *,unsigned,const AWRecord *,unsigned);
enum AWStatus aw_init(AWOwner *,unsigned,unsigned,unsigned,AWContext *,unsigned,AWCompare,void *,AWBacking);
enum AWStatus aw_put(AWOwner *,const AWRecord *);
enum AWStatus aw_remove(AWOwner *,int32_t);
enum AWStatus aw_reserve(AWOwner *,unsigned);
enum AWStatus aw_set_context(AWOwner *,AWContext *);
enum AWStatus aw_view(const AWOwner *,unsigned,AWView *);
enum AWStatus aw_read(AWView,AWRecord *);
enum AWStatus aw_destroy(AWOwner *);
int aw_finite_compare(const AWRecord *,const AWRecord *,unsigned);
int aw_callback_compare(AWCompare,void *,const AWRecord *,const AWRecord *,int *);
int aw_test(const char *);
void aw_observe(const char *,const int *,unsigned);
void aw_failure(const char *,unsigned);
#endif
