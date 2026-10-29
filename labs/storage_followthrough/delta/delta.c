#include "delta.h"

int sd_format(store_io i)
{
    (void)i;
    return SD_UNIMPLEMENTED;
}

int sd_open(struct sd_store *s, store_io i, unsigned f)
{
    (void)s;
    (void)i;
    (void)f;
    return SD_UNIMPLEMENTED;
}

int sd_begin(struct sd_store *s)
{
    (void)s;
    return SD_UNIMPLEMENTED;
}

int sd_write(struct sd_store *s, unsigned p, size_t o, const void *b, size_t n)
{
    (void)s;
    (void)p;
    (void)o;
    (void)b;
    (void)n;
    return SD_UNIMPLEMENTED;
}

int sd_read(struct sd_store *s, unsigned p, void *b)
{
    (void)s;
    (void)p;
    (void)b;
    return SD_UNIMPLEMENTED;
}

int sd_commit(struct sd_store *s)
{
    (void)s;
    return SD_UNIMPLEMENTED;
}

int sd_checkpoint(struct sd_store *s)
{
    (void)s;
    return SD_UNIMPLEMENTED;
}

void sd_abort(struct sd_store *s)
{
    (void)s;
}
