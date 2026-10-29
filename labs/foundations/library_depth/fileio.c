#include "fileio.h"

void li_init(struct li_copy *c)
{
    if (c)
    {
        *c = (struct li_copy){0};
    }
}

enum li_status li_pump(struct li_copy *c, LIRead r, void *rc, f_writer_fn w,
                       void *wc, unsigned budget)
{
    (void)c;
    (void)r;
    (void)rc;
    (void)w;
    (void)wc;
    (void)budget;
    return LI_NOT_IMPLEMENTED;
}

enum li_status li_release(struct li_owner *o, int (*f)(void *),
                          int (*c)(void *))
{
    (void)o;
    (void)f;
    (void)c;
    return LI_NOT_IMPLEMENTED;
}
