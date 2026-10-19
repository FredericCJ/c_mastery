#ifndef CORPUS_ERASE_H
#define CORPUS_ERASE_H
#include <stddef.h>
#include <stdint.h>
enum ErStatus { ER_OK,ER_INVALID,ER_FAILED,ER_STALE,ER_EXHAUSTED };
typedef struct { unsigned id; int value; } ErItem;
typedef struct { ErItem items[12]; size_t count,keep; uint32_t epoch; unsigned pending; } ErOwner;
typedef struct { const ErOwner *owner; uint32_t epoch; size_t new_end; } ErTail;
typedef int (*ErPredicate)(void *,const ErItem *);
typedef void (*ErDrop)(void *,unsigned);
int er_init(ErOwner *,const ErItem *,size_t);
int er_compact(int *,size_t,int,size_t *,size_t *);
int er_remove_if(ErOwner *,ErPredicate,void *,ErTail *);
int er_erase(ErOwner *,ErTail,ErDrop,void *);
int er_clear(ErOwner *,ErDrop,void *);
#endif
