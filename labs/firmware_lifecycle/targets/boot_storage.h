#ifndef B06_BOOT_STORAGE_H
#define B06_BOOT_STORAGE_H
#include "boot_pair.h"
#include "ecc_guard.h"
typedef struct {uint32_t sequence;unsigned record;FBImage image[2];} BSSelection;
typedef struct {FBLayout core[2][2];} BSLayouts;
/* Two admitted physical slot choices per core. Header lies32bytes before
 * layout.base; signature64bytes at base+2048. Full slots are immutable/owned.
 * Only readable, authenticated, current-policy paired records participate. */
int bs_read_image(EGRead,void *,const FBPolicy *,FBLayout,unsigned core,FBImage *);
int bs_select(EGRead,void *,const FBPolicy *,const BSLayouts *,
              const uint32_t record_bases[2],BSSelection *);
#endif
