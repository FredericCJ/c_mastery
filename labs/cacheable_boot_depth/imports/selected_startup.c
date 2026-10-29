#include "selected_startup.h"

int st_cold_h745(struct fh_io x)
{
    (void)x;
    return 0;
}

int st_cold_f439(struct fh_io x)
{
    (void)x;
    return 0;
}

int st_release(struct fh_io x, const struct bs_selection *s)
{
    (void)x;
    (void)s;
    return 0;
}

int st_peer_matches(struct fh_io x, const struct bs_selection *s)
{
    (void)x;
    (void)s;
    return 0;
}
