#ifndef CMASTERY_VM_SERVICE_MEDIUM_H
#define CMASTERY_VM_SERVICE_MEDIUM_H
#include "store.h"
/* Original bounded teaching device. One owned image, synchronous full-page
 * callbacks. A failure may persist all pending bytes yet lose its reply. */
typedef struct {
  uint8_t stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
  uint8_t pending[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
  uint8_t dirty[STORE_IMAGE_PAGES];
  unsigned event, cut, persist_on_failure, dead;
} VhMedium;
store_io vh_medium_io(VhMedium *);
void vh_medium_restart(VhMedium *);
#endif
