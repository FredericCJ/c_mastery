/* Bounded proof harness: assumptions and reachability are audited separately. */
#include "core.h"
#include <assert.h>
extern unsigned nondet_unsigned(void);
extern uint32_t nondet_uint32_t(void);

int main(void)
{
    uint32_t a[8], key = nondet_uint32_t();
    unsigned n = nondet_unsigned(), i;
    size_t pos;
    struct vf_ring r;
    __CPROVER_assume(n <= 8);
    for (i = 0; i < n; ++i)
    {
        a[i] = nondet_uint32_t();
        if (i)
            __CPROVER_assume(a[i - 1] <= a[i]);
    }
#ifdef VACUOUS
    __CPROVER_assume(n > 8);
#endif
#ifdef WITNESS
    assert(0 && "assumption domain is reachable");
#endif
    pos = vf_lower_bound(a, n, key);
    assert(pos <= n);
    for (i = 0; i < pos; ++i)
        assert(a[i] < key);
    for (i = (unsigned)pos; i < n; ++i)
        assert(a[i] >= key);
    r.capacity = nondet_unsigned();
    r.head = nondet_unsigned();
    r.size = nondet_unsigned();
    __CPROVER_assume(r.capacity >= 1 && r.capacity <= 8 &&
                     r.head < r.capacity && r.size <= r.capacity);
    for (i = 0; i < 8; ++i)
    {
        struct vf_ring old = r;
        unsigned op = nondet_unsigned();
        int ok;
        __CPROVER_assume(op <= 1);
        ok = vf_ring_step(&r, op);
        assert(r.head < r.capacity && r.size <= r.capacity);
        assert(ok == (op == 0 ? old.size < old.capacity : old.size > 0));
        if (!ok)
        {
            assert(r.head == old.head && r.size == old.size);
        }
        else if (op == 0)
        {
            assert(r.size == old.size + 1 && r.head == old.head);
        }
        else
        {
            assert(r.size == old.size - 1 &&
                   r.head == (old.head + 1 == old.capacity ? 0 : old.head + 1));
        }
    }
    return 0;
}
