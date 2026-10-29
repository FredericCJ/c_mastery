#include "formal.h"
#include <assert.h>
extern unsigned nondet_unsigned(void);
extern _Bool nondet_bool(void);
void __CPROVER_assume(_Bool);

struct environment
{
    const unsigned char *base;
    size_t accepted;
    unsigned calls;
};

static ptrdiff_t writer(void *v, const unsigned char *p, size_t remaining)
{
    struct environment *e = v;
    unsigned choice = nondet_unsigned();
    __CPROVER_assume(choice < 4);
    assert(p == e->base + e->accepted);
    assert(remaining == 4 - e->accepted);
    ++e->calls;
    ptrdiff_t result = choice == 0 ? -1 : choice == 1 ? 0 : choice == 2 ? 1 : 5;
    if (result > 0 && (size_t)result <= remaining)
        e->accepted += (size_t)result;
    return result;
}

int main(void)
{
    unsigned char allocated[4] = {1, 2, 3, 4};
    _Bool acquired = nondet_bool();
    const unsigned char *p =
        acquired ? &allocated[0] : (const unsigned char *)0;
    size_t committed = 99;
    struct environment e = {allocated, 0, 0};
    enum bf_status s = bf_write(writer, &e, p, 4, 3, &committed);
    if (!acquired)
    {
        assert(s == BF_INVALID);
        assert(committed == 99);
        assert(e.calls == 0);
    }
    else
    {
        assert(committed == e.accepted);
        assert(committed <= 4);
        assert(e.calls <= 3);
    }
    return 0;
}
