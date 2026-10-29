#ifndef B06_F439_UPDATE_H
#define B06_F439_UPDATE_H
#include "recovery_transport.h"

struct f4_selection
{
    uint32_t sequence;
    unsigned record;
    struct fb_image image;
};

struct f4_writer
{
    struct fm_io io;
    uint32_t base, sector_base, control, checked;
    unsigned length, word, state, mode;
    unsigned char bytes[2144];
};

/* Selected two-MiB unswapped F439, x32 at qualified2.7..3.6V, bank2 code.
 * All buffers distinct, callbacks live/nonreentrant, exclusive flash ownership.
 * Cold selector never consults trial sector7. No invented second core. */
int f4_record(const struct f4_selection *, unsigned char out[80]);
int f4_select(eg_read_fn, void *, const struct fb_policy *,
              const struct fb_layout choices[2], struct f4_selection *);
enum fh_result f4_install_begin(struct f4_writer *, struct fm_io,
                                const struct f4_selection *,
                                const struct rt_frame *,
                                const struct fb_policy *,
                                const struct fb_layout choices[2]);
int f4_install_result(const struct f4_writer *, eg_read_fn, void *,
                      const struct fb_policy *,
                      const struct fb_layout choices[2], struct fb_image *);
enum fh_result f4_trial_begin(struct f4_writer *, struct fm_io,
                              const struct f4_selection *,
                              const struct fb_image *);
int f4_trial_probe(eg_read_fn, void *, const struct fb_policy *,
                   const struct fb_layout choices[2],
                   const struct f4_selection *, struct f4_selection *);
enum fh_result f4_commit_begin(struct f4_writer *, struct fm_io,
                               const struct f4_selection *,
                               const struct f4_selection *,
                               uint32_t application_generation,
                               uint32_t application_result);
enum fh_result f4_poll(struct f4_writer *);
#endif
