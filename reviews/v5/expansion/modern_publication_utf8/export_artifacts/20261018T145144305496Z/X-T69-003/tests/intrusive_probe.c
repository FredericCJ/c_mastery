#include "dispatch_intrusive.h"
#include <stdio.h>
/* Tested GNU/Clang conventional container recovery; not a proof for arbitrary pointers. */
DSDualItem *probe_offset(DSHook *hook) {
  return (DSDualItem *)((unsigned char *)hook - offsetof(DSDualItem, hooks[1]));
}
DSDualItem *probe_parent(DSHook *hook) { return hook->parent; }
DSItem *probe_initial(DSLink *hook) { return (DSItem *)hook; }
int main(void) {
  DSDualItem item;DSItem first;ds_dual_item_init(&item,7);
  if(probe_parent(&item.hooks[1])!=&item || probe_offset(&item.hooks[1])!=&item || probe_initial(&first.link)!=&first)return 1;
  printf("%zu %zu %zu %zu %zu %zu %zu %zu\n",sizeof(DSDualItem),sizeof(DSHook),sizeof(DSDualList),sizeof(DSIndexOwner),sizeof(DSIndexSlot),sizeof(DSIndexHandle),offsetof(DSDualItem,hooks[1]),_Alignof(DSDualItem));
  return 0;
}
