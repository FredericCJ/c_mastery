#ifndef B06_BOOT_STORAGE_H
#define B06_BOOT_STORAGE_H
#include "boot_pair.h"
#include "ecc_guard.h"

struct bs_selection
{
    uint32_t sequence;
    unsigned record;
    struct fb_image image[2];
};

struct bs_layouts
{
    struct fb_layout core[2][2];
};

/* Two admitted physical slot choices per core. Header lies32bytes before
 * layout.base; signature64bytes at base+2048. Full slots are immutable/owned.
 * Only readable, authenticated, current-policy paired records participate. */
int bs_read_image(eg_read_fn, void *, const struct fb_policy *,
                  struct fb_layout, unsigned core, struct fb_image *);
int bs_select(eg_read_fn, void *, const struct fb_policy *,
              const struct bs_layouts *, const uint32_t record_bases[2],
              struct bs_selection *);
#endif
