#ifndef CMASTERY_DISPATCH_H
#define CMASTERY_DISPATCH_H
#include "dod.h"
#include <stddef.h>
enum DSStatus {
  DS_OK,
  DS_INVALID,
  DS_RANGE,
  DS_EMPTY,
  DS_BUSY,
  DS_FULL,
  DS_FAILED,
  DS_DENIED,
  DS_STALE,
  DS_UNSUPPORTED
};
typedef struct {
  int (*measure)(const void *, int *);
} DSOps;
typedef struct {
  const void *object;
  const DSOps *ops;
} DSView;
typedef struct {
  const DSOps *ops;
  int value;
} DSBox;
int ds_measure(DSView, int *);
int ds_embedded(const DSBox *, int *);
typedef int (*DSCombine)(int, int, int *);
int ds_fold(const int *, size_t, int, DSCombine, int *);
int ds_sum(const int *, size_t, int, int *);
enum DSMode { DS_SHARED, DS_MUTABLE, DS_ONCE };
typedef struct {
  int capture;
  enum DSMode mode;
  unsigned active;
} DSClosure;
int ds_closure_init(DSClosure *, int, enum DSMode);
int ds_closure_call(DSClosure *, int, int *);
typedef int (*DSThunk)(const unsigned char *, size_t, int, int *);
typedef struct {
  DSmall capture;
  DSThunk call;
  void (*destroy)(void *);
  void *resource;
  unsigned active;
} DSErased;
void ds_erased_init(DSErased *, DAllocator);
int ds_erased_assign(DSErased *, const unsigned char *, size_t, DSThunk,
                     void (*)(void *), void *);
int ds_erased_call(const DSErased *, int, int *);
int ds_erased_move(DSErased *, DSErased *);
void ds_erased_drop(DSErased *);
typedef struct {
  DSCombine entries[3][3];
  unsigned epoch;
} DSMethods;
typedef struct {
  unsigned epoch, left, right;
  DSCombine method;
} DSCache;
void ds_methods_init(DSMethods *);
int ds_methods_set(DSMethods *, unsigned, unsigned, DSCombine);
int ds_dispatch(const DSMethods *, DSCache *, unsigned, unsigned, int, int,
                int *);
typedef struct DSList DSList;
typedef struct DSLink {
  struct DSLink *previous, *next;
  DSList *owner;
} DSLink;
typedef struct {
  DSLink link;
  int value;
} DSItem;
struct DSList {
  DSLink head;
  unsigned count;
};
void ds_list_init(DSList *);
void ds_item_init(DSItem *, int);
int ds_list_push(DSList *, DSItem *);
int ds_list_remove(DSList *, DSItem *);
int ds_list_sum(const DSList *, int *);
typedef struct DSModule DSModule;
typedef struct {
  int (*read)(void *, int *);
  void *context;
} DSService;
int ds_module_open(DSService, DSModule **);
int ds_module_twice(DSModule *, int *);
void ds_module_close(DSModule *);
typedef struct {
  unsigned rights, epoch;
  int counter;
} DSAuthority;
typedef struct {
  const DSAuthority *owner;
  unsigned rights, epoch;
} DSCapability;
void ds_authority_init(DSAuthority *, unsigned);
int ds_cap_issue(const DSAuthority *, unsigned, DSCapability *);
int ds_cap_attenuate(DSCapability, unsigned, DSCapability *);
int ds_cap_add(DSAuthority *, DSCapability, int);
int ds_cap_revoke(DSAuthority *);
typedef struct {
  unsigned abi;
  size_t size;
  unsigned schema;
  int (*apply)(int, int, int *);
  int (*migrate)(unsigned, int, int *);
} DSPlugin;
typedef struct {
  void *handle;
  const DSPlugin *api;
  unsigned borrows;
} DSLoaded;
void ds_loaded_init(DSLoaded *);
int ds_loaded_open(DSLoaded *, const char *);
int ds_loaded_acquire(DSLoaded *, const DSPlugin **);
int ds_loaded_release(DSLoaded *);
int ds_loaded_close(DSLoaded *);
typedef struct {
  DSLoaded current, retired;
  int state;
  unsigned generation;
} DSHost;
void ds_host_init(DSHost *);
int ds_host_reload(DSHost *, DSLoaded *);
int ds_host_collect(DSHost *);
#endif
