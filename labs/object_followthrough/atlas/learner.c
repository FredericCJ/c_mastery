#include "atlas.h"

int oa_resize(struct oa_owner *a, size_t b, const struct oa_allocator *c)
{
    (void)a;
    (void)b;
    (void)c;
    return -1;
}

void oa_release(struct oa_owner *a, const struct oa_allocator *b)
{
    (void)a;
    (void)b;
}

int oa_label(char *a, size_t b, struct oa_view *c)
{
    (void)a;
    (void)b;
    (void)c;
    return -1;
}

int oa_deep_copy(struct oa_view a, char *b, size_t c, struct oa_view *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

int oa_equal(const struct oa_record *a, const struct oa_record *b)
{
    (void)a;
    (void)b;
    return 0;
}

int oa_encode(const struct oa_record *a, unsigned char *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return -1;
}

int oa_place(size_t a, size_t b, size_t c, size_t d, size_t e, size_t *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return -1;
}

int oa_slot(struct oa_storage *a, size_t b, size_t c, struct oa_record **d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

int oa_wide_admit(size_t a)
{
    (void)a;
    return 0;
}

int oa_decode_u32(const unsigned char *a, size_t b, uint32_t *c)
{
    (void)a;
    (void)b;
    (void)c;
    return -1;
}
