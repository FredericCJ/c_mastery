#ifndef CMASTERY_DISPATCH_INTRUSIVE_H
#define CMASTERY_DISPATCH_INTRUSIVE_H
#include "dispatch.h"
typedef struct DSDualItem DSDualItem;
typedef struct DSDualList DSDualList;
typedef struct DSHook {
  struct DSHook *previous, *next;
  DSDualList *owner;
  DSDualItem *parent;
  unsigned kind;
} DSHook;
struct DSDualItem { int value; DSHook hooks[2]; };
struct DSDualList { DSHook head; unsigned kind, count; };
void ds_dual_init(DSDualList *, unsigned);
void ds_dual_item_init(DSDualItem *, int);
int ds_dual_push(DSDualList *, DSDualItem *);
int ds_dual_remove(DSDualList *, DSDualItem *);
int ds_dual_fold(const DSDualList *, DSCombine, int, int *);
#define DS_INDEX_NONE 4u
typedef struct { int value; unsigned previous[2], next[2], linked[2]; } DSIndexSlot;
typedef struct DSIndexOwner {
  DSIndexSlot *slots;
  unsigned live[4], generation[4], head[2], tail[2], count[2];
} DSIndexOwner;
typedef struct { const DSIndexOwner *owner; unsigned index, generation; } DSIndexHandle;
void ds_index_init(DSIndexOwner *, DSIndexSlot [4]);
int ds_index_new(DSIndexOwner *, int, DSIndexHandle *);
int ds_index_get(const DSIndexOwner *, DSIndexHandle, int *);
int ds_index_push(DSIndexOwner *, unsigned, DSIndexHandle);
int ds_index_remove(DSIndexOwner *, unsigned, DSIndexHandle);
int ds_index_release(DSIndexOwner *, DSIndexHandle);
int ds_index_relocate(DSIndexOwner *, DSIndexSlot [4]);
int ds_index_fold(const DSIndexOwner *, unsigned, DSCombine, int, int *);
#endif
