#ifndef CORPUS_ERASE_H
#define CORPUS_ERASE_H
#include <stddef.h>
#include <stdint.h>
enum er_status { ER_OK,ER_INVALID,ER_FAILED,ER_STALE,ER_EXHAUSTED };
struct er_item { unsigned id; int value; };
struct er_owner { struct er_item items[12]; size_t count,keep; uint32_t epoch; unsigned pending; };
struct er_tail { const struct er_owner *owner; uint32_t epoch; size_t new_end; };
typedef int (*er_predicate_fn)(void *,const struct er_item *);
typedef void (*er_drop_fn)(void *,unsigned);
int er_init(struct er_owner *,const struct er_item *,size_t);
int er_compact(int *,size_t,int,size_t *,size_t *);
int er_remove_if(struct er_owner *,er_predicate_fn,void *,struct er_tail *);
int er_erase(struct er_owner *,struct er_tail,er_drop_fn,void *);
int er_clear(struct er_owner *,er_drop_fn,void *);
#endif
