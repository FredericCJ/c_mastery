#ifndef ALGORITHM_WRAPPERS_H
#define ALGORITHM_WRAPPERS_H
#include <stddef.h>
#include <stdint.h>
#include "containers.h"
struct aw_record { int32_t id,rank; };
struct aw_context { unsigned epoch,descending; };
struct aw_bank { struct aw_record values[3]; const struct aw_record *pointers[3]; };
typedef int (*aw_compare_fn)(void *,const struct aw_record *,const struct aw_record *,int *);
struct aw_backing { void *context; struct aw_bank *(*acquire)(void *); void (*release)(void *,struct aw_bank *); };
enum aw_status { AW_OK,AW_INVALID,AW_FULL,AW_DUPLICATE,AW_MISSING,AW_BACKING,AW_COMPARE,AW_BUSY,AW_STALE,AW_MUTATED,AW_EXHAUSTED };
enum aw_kind { AW_VALUE,AW_BORROW };
enum aw_dispatch { AW_FINITE,AW_CALLBACK };
struct aw_owner {
 struct cd_tree ids;
 struct aw_bank *bank;
 struct aw_record expected[3];
 struct aw_backing backing;
 struct aw_context *context;
 aw_compare_fn compare;
 void *compare_context;
 unsigned count,capacity,generation,limit,context_epoch,descending,kind,dispatch,alive,busy;
};
struct aw_view { const struct aw_owner *owner; unsigned generation,index; };
/* The kernel requires sorted actual Record objects, count<=3, direction0/1.
 * It returns the first position not less than query in rank/ID order. */
unsigned aw_lower(const struct aw_record *,unsigned,const struct aw_record *,unsigned);
unsigned aw_probe(const struct aw_record *,unsigned,const struct aw_record *,unsigned);
enum aw_status aw_init(struct aw_owner *,unsigned,unsigned,unsigned,struct aw_context *,unsigned,aw_compare_fn,void *,struct aw_backing);
enum aw_status aw_put(struct aw_owner *,const struct aw_record *);
enum aw_status aw_remove(struct aw_owner *,int32_t);
enum aw_status aw_reserve(struct aw_owner *,unsigned);
enum aw_status aw_set_context(struct aw_owner *,struct aw_context *);
enum aw_status aw_view(const struct aw_owner *,unsigned,struct aw_view *);
enum aw_status aw_read(struct aw_view,struct aw_record *);
enum aw_status aw_destroy(struct aw_owner *);
int aw_finite_compare(const struct aw_record *,const struct aw_record *,unsigned);
int aw_callback_compare(aw_compare_fn,void *,const struct aw_record *,const struct aw_record *,int *);
int aw_test(const char *);
void aw_observe(const char *,const int *,unsigned);
void aw_failure(const char *,unsigned);
#endif
