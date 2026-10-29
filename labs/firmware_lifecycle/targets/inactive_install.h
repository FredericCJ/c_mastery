#ifndef B06_INACTIVE_INSTALL_H
#define B06_INACTIVE_INSTALL_H
#include "recovery_transport.h"

struct fu_install
{
    struct fm_io io;
    eg_read_fn read;
    void *read_ctx;
    struct fb_policy policy;
    struct fb_layout layout;
    struct fb_image expected;
    struct fh_session program;
    uint32_t sector_base, control, checked;
    unsigned state, word;
    unsigned char bytes[2144];
};
/* Fresh all-zero control object; current-policy/old selection trusted and owned.
 * Only inactive H745 bank1 sectors1..4, never bootstrap/log/active slot. */
enum fh_result fu_begin(struct fu_install *, struct fm_io, eg_read_fn, void *,
                        const struct rt_frame *, const struct fb_policy *,
                        const struct bs_layouts *,
                        const struct bs_selection *old);
/* At most32 erase-verification words per poll; one program-word transition.
 * Output unchanged until successful fresh signed storage readback. FH_DONE
 * means inactive image installed, not trial-health or committed activation. */
enum fh_result fu_poll(struct fu_install *, struct fb_image *);
#endif
