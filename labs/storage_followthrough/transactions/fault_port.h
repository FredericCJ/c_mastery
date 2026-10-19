#ifndef STORAGE_FOLLOW_FAULT_PORT_H
#define STORAGE_FOLLOW_FAULT_PORT_H
#include <stdint.h>
/* Explicit host-laboratory verification port, not an allocator library.
   Call immediately before marking an available typed node allocated. False
   admits no allocation; it must not acquire external resources. Nonreentrant.
   refresh_visit is observation-only at each actual full-refresh node visit. */
int st_page_admit(uint32_t logical,int leaf);
void st_refresh_visit(uint32_t logical);
#endif
