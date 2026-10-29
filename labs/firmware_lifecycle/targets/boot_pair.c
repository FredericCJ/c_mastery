#include "boot_pair.h"

int fb_admit(const struct fb_policy *a, struct fb_layout b, unsigned c,
             const unsigned char *d, size_t e, const unsigned char f[64],
             struct fb_image *g)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    return 0;
}

int fb_joint_record(const struct fb_image a[2], const struct fw_peer *b,
                    uint32_t c, unsigned char d[128])
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 0;
}

int fb_record_matches(const unsigned char a[128], const struct fb_image b[2])
{
    (void)a;
    (void)b;
    return 0;
}
