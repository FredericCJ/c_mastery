#include "lc.h"
#include "test.h"

struct ac_old
{
    struct lc_buffer *p;
    size_t n;
};

struct ac_named
{
    struct lc_buffer *owned_buffer;
    struct lc_bytes capacity_bytes;
};

static enum lc_status old_append(struct ac_old *p, const unsigned char *s,
                                 size_t size)
{
    return lc_append(p->p, s, size);
}

static enum lc_status named_append(struct ac_named *p,
                                   const unsigned char *borrowed,
                                   size_t length_bytes)
{
    return lc_append(p->owned_buffer, borrowed, length_bytes);
}

int main(void)
{
    unsigned char x[] = {9, 0, 7};
    struct ac_old a = {lc_create(3), 3};
    struct ac_named b = {lc_create(3), {3}};
    CHECK(a.p && b.owned_buffer);
    CHECK(old_append(&a, x, 3) == named_append(&b, x, 3));
    CHECK(old_append(&a, x, 1) == named_append(&b, x, 1));
    size_t an = 0, bn = 0;
    const unsigned char *av = lc_view(a.p, &an),
                        *bv = lc_view(b.owned_buffer, &bn);
    CHECK(an == bn);
    for (size_t i = 0; i < an; i++)
        CHECK(av[i] == bv[i]);
    lc_release_buffer(a.p);
    lc_release_buffer(b.owned_buffer);
    puts("LC_RENAME unchanged byte units and ownership");
    return 0;
}
