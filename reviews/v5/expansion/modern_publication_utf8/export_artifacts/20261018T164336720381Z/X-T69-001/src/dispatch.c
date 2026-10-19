#include "dispatch.h"
/* Learner mechanisms intentionally incomplete; empty-state initialization
 * supplied. */
int ds_measure(DSView arg0, int *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_embedded(const DSBox *arg0, int *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_fold(const int *arg0, size_t arg1, int arg2, DSCombine arg3, int *arg4) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  return DS_INVALID;
}
int ds_sum(const int *arg0, size_t arg1, int arg2, int *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return DS_INVALID;
}
int ds_closure_init(DSClosure *arg0, int arg1, enum DSMode arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_closure_call(DSClosure *arg0, int arg1, int *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
void ds_erased_init(DSErased *e, DAllocator a) {
  d_small_init(&e->capture, a);
  e->call = 0;
  e->destroy = 0;
  e->resource = 0;
  e->active = 0;
}
int ds_erased_assign(DSErased *arg0, const unsigned char *arg1, size_t arg2,
                     DSThunk arg3, void (*arg4)(void *), void *arg5) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  return DS_INVALID;
}
int ds_erased_call(const DSErased *arg0, int arg1, int *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_erased_move(DSErased *arg0, DSErased *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
void ds_erased_drop(DSErased *arg0) { (void)arg0; }
void ds_methods_init(DSMethods *m) {
  unsigned i, j;
  m->epoch = 1;
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      m->entries[i][j] = 0;
}
int ds_methods_set(DSMethods *arg0, unsigned arg1, unsigned arg2,
                   DSCombine arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return DS_INVALID;
}
int ds_dispatch(const DSMethods *arg0, DSCache *arg1, unsigned arg2,
                unsigned arg3, int arg4, int arg5, int *arg6) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  (void)arg6;
  return DS_INVALID;
}
void ds_list_init(DSList *l) {
  l->head = (DSLink){&l->head, &l->head, l};
  l->count = 0;
}
void ds_item_init(DSItem *i, int value) {
  i->link = (DSLink){0, 0, 0};
  i->value = value;
}
int ds_list_push(DSList *arg0, DSItem *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_list_remove(DSList *arg0, DSItem *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_list_sum(const DSList *arg0, int *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_module_open(DSService arg0, DSModule **arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_module_twice(DSModule *arg0, int *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
void ds_module_close(DSModule *arg0) { (void)arg0; }
void ds_authority_init(DSAuthority *a, unsigned rights) {
  a->rights = rights & 3;
  a->epoch = 1;
  a->counter = 0;
}
int ds_cap_issue(const DSAuthority *arg0, unsigned arg1, DSCapability *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_cap_attenuate(DSCapability arg0, unsigned arg1, DSCapability *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_cap_add(DSAuthority *arg0, DSCapability arg1, int arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_cap_revoke(DSAuthority *arg0) {
  (void)arg0;
  return DS_INVALID;
}
void ds_loaded_init(DSLoaded *m) { *m = (DSLoaded){0, 0, 0}; }
int ds_loaded_open(DSLoaded *arg0, const char *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_loaded_acquire(DSLoaded *arg0, const DSPlugin **arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_loaded_release(DSLoaded *arg0) {
  (void)arg0;
  return DS_INVALID;
}
int ds_loaded_close(DSLoaded *arg0) {
  (void)arg0;
  return DS_INVALID;
}
void ds_host_init(DSHost *h) {
  ds_loaded_init(&h->current);
  ds_loaded_init(&h->retired);
  h->state = 0;
  h->generation = 0;
}
int ds_host_reload(DSHost *arg0, DSLoaded *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_host_collect(DSHost *arg0) {
  (void)arg0;
  return DS_INVALID;
}
