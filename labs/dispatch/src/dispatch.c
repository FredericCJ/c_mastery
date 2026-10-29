#include "dispatch.h"
/* Learner mechanisms intentionally incomplete; empty-state initialization
 * supplied. */
int ds_measure(struct ds_view arg0, int *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_embedded(const struct ds_box *arg0, int *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_fold(const int *arg0, size_t arg1, int arg2, ds_combine_fn arg3, int *arg4) {
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
int ds_closure_init(struct ds_closure *arg0, int arg1, enum ds_mode arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_closure_call(struct ds_closure *arg0, int arg1, int *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
void ds_erased_init(struct ds_erased *e, struct d_allocator a) {
  d_small_init(&e->capture, a);
  e->call = 0;
  e->destroy = 0;
  e->resource = 0;
  e->active = 0;
}
int ds_erased_assign(struct ds_erased *arg0, const unsigned char *arg1, size_t arg2,
                     ds_thunk_fn arg3, void (*arg4)(void *), void *arg5) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  return DS_INVALID;
}
int ds_erased_call(const struct ds_erased *arg0, int arg1, int *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_erased_move(struct ds_erased *arg0, struct ds_erased *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
void ds_erased_drop(struct ds_erased *arg0) { (void)arg0; }
void ds_methods_init(struct ds_methods *m) {
  unsigned i, j;
  m->epoch = 1;
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      m->entries[i][j] = 0;
}
int ds_methods_set(struct ds_methods *arg0, unsigned arg1, unsigned arg2,
                   ds_combine_fn arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return DS_INVALID;
}
int ds_dispatch(const struct ds_methods *arg0, struct ds_cache *arg1, unsigned arg2,
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
void ds_list_init(struct ds_list *l) {
  l->head = (struct ds_link){&l->head, &l->head, l};
  l->count = 0;
}
void ds_item_init(struct ds_item *i, int value) {
  i->link = (struct ds_link){0, 0, 0};
  i->value = value;
}
int ds_list_push(struct ds_list *arg0, struct ds_item *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_list_remove(struct ds_list *arg0, struct ds_item *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_list_sum(const struct ds_list *arg0, int *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_module_open(struct ds_service arg0, struct ds_module **arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_module_twice(struct ds_module *arg0, int *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
void ds_module_close(struct ds_module *arg0) { (void)arg0; }
void ds_authority_init(struct ds_authority *a, unsigned rights) {
  a->rights = rights & 3;
  a->epoch = 1;
  a->counter = 0;
}
int ds_cap_issue(const struct ds_authority *arg0, unsigned arg1, struct ds_capability *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_cap_attenuate(struct ds_capability arg0, unsigned arg1, struct ds_capability *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_cap_add(struct ds_authority *arg0, struct ds_capability arg1, int arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return DS_INVALID;
}
int ds_cap_revoke(struct ds_authority *arg0) {
  (void)arg0;
  return DS_INVALID;
}
void ds_loaded_init(struct ds_loaded *m) { *m = (struct ds_loaded){0, 0, 0}; }
int ds_loaded_open(struct ds_loaded *arg0, const char *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_loaded_acquire(struct ds_loaded *arg0, const struct ds_plugin **arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_loaded_release(struct ds_loaded *arg0) {
  (void)arg0;
  return DS_INVALID;
}
int ds_loaded_close(struct ds_loaded *arg0) {
  (void)arg0;
  return DS_INVALID;
}
void ds_host_init(struct ds_host *h) {
  ds_loaded_init(&h->current);
  ds_loaded_init(&h->retired);
  h->state = 0;
  h->generation = 0;
}
int ds_host_reload(struct ds_host *arg0, struct ds_loaded *arg1) {
  (void)arg0;
  (void)arg1;
  return DS_INVALID;
}
int ds_host_collect(struct ds_host *arg0) {
  (void)arg0;
  return DS_INVALID;
}
