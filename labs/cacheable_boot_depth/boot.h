#ifndef CACHEABLE_BOOT_DEPTH_H
#define CACHEABLE_BOOT_DEPTH_H
#include <stdint.h>
#include "selected_startup.h"
typedef struct { FHIO registers; void (*isb)(void *); } CBIO;
/* Cold, exclusive, privileged, nonreentrant register callbacks. No rollback
 * after a hardware fault. g must be a nonzero trusted current boot generation. */
int cb_boot(CBIO, uint32_t g);
/* Released M4 only: configure its noncached map, never clear the shared line. */
int cb_peer(CBIO, uint32_t g);
/* Exact isolated 32-byte region. C does not confer physical cache semantics. */
#define CB_LINE UINT32_C(0x30040000)
#define CB_M7_RASR UINT32_C(0x110B0009)
#define CB_M4_RASR UINT32_C(0x110C0009)
CBIO cb_target_io(void);
#endif
