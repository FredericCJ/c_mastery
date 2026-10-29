#include "dispatch_intrusive.h"
#include <stdio.h>
/* Tested GNU/Clang conventional container recovery; not a proof for arbitrary pointers. */
struct ds_dual_item *probe_offset(struct ds_hook *hook) {
  return (struct ds_dual_item *)((unsigned char *)hook - offsetof(struct ds_dual_item, hooks[1]));
}
struct ds_dual_item *probe_parent(struct ds_hook *hook) { return hook->parent; }
struct ds_item *probe_initial(struct ds_link *hook) { return (struct ds_item *)hook; }
int main(void) {
  struct ds_dual_item item;struct ds_item first;ds_dual_item_init(&item,7);
  if(probe_parent(&item.hooks[1])!=&item || probe_offset(&item.hooks[1])!=&item || probe_initial(&first.link)!=&first)return 1;
  printf("%zu %zu %zu %zu %zu %zu %zu %zu\n",sizeof(struct ds_dual_item),sizeof(struct ds_hook),sizeof(struct ds_dual_list),sizeof(struct ds_index_owner),sizeof(struct ds_index_slot),sizeof(struct ds_index_handle),offsetof(struct ds_dual_item,hooks[1]),_Alignof(struct ds_dual_item));
  return 0;
}
