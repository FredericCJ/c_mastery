#ifndef STORAGE_FOLLOW_DURABILITY_MEDIUM_H
#define STORAGE_FOLLOW_DURABILITY_MEDIUM_H
#include "store.h"
enum {DM_NONE=0,DM_EVEN=1,DM_PREFIX=2,DM_SUFFIX=3,DM_ALL=4,DM_ODD=5,DM_MIDDLE=6,DM_SUBSET=7};
#ifndef DM_PAGES
#define DM_PAGES 67
#endif
_Static_assert(DM_PAGES==67||DM_PAGES==68,"explicit two/three checkpoint profiles");
struct dm_event {unsigned kind,page;};
struct d_medium {uint8_t stable[DM_PAGES][4096],pending[DM_PAGES][4096],dirty[DM_PAGES];uint64_t reads,writes,flushes;unsigned event,cut,mode,lie_flush,read_cut;int dead,short_read;uint64_t subset[2];struct dm_event trace[128];};
/* Original local model only. Stable/pending are distinct; successful writes
 * stage acceptance, successful nonlying flush persists every pending page.
 * A selected error boundary may persist an explicitly chosen subset or tear.
 * The lying-flush option is a separate violated-premise investigation. */
void dm_init(struct d_medium *);
int dm_arm(struct d_medium *,unsigned cut,unsigned mode,unsigned lie_flush,uint64_t low,uint64_t high);
void dm_reboot(struct d_medium *);
int dm_read(void *,uint32_t,void *);
int dm_write(void *,uint32_t,const void *);
int dm_flush(void *);
store_io dm_io(struct d_medium *);
#endif
