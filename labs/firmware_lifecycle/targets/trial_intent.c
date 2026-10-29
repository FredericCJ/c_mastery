#include "trial_intent.h"

int ft_intent(const struct bs_selection *o, const struct fb_image i[2],
              unsigned char b[128])
{
    (void)o;
    (void)i;
    (void)b;
    return 0;
}

int ft_attempt(const unsigned char i[128], unsigned char b[32])
{
    (void)i;
    (void)b;
    return 0;
}

int ft_probe(eg_read_fn r, void *c, const struct fb_policy *p,
             const struct bs_layouts *l, const struct bs_selection *o,
             struct bs_selection *out)
{
    (void)r;
    (void)c;
    (void)p;
    (void)l;
    (void)o;
    (void)out;
    return 0;
}

enum fh_result ft_begin(struct ft_writer *w, struct fm_io io,
                        const struct bs_selection *o,
                        const struct fb_image i[2])
{
    (void)w;
    (void)io;
    (void)o;
    (void)i;
    return FH_REJECT;
}

enum fh_result ft_poll(struct ft_writer *w)
{
    (void)w;
    return FH_REJECT;
}
