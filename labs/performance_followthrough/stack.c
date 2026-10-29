/* Compiler-observation specimen, GNU host extension for alignment only. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
static volatile unsigned sink;

__attribute__((noinline)) unsigned pf_static(unsigned n)
{
    volatile unsigned a[8] = {0};
    a[n & 7] = n;
    return a[n & 7];
}

__attribute__((noinline)) unsigned pf_bounded(unsigned n)
{
    volatile unsigned char a[32] __attribute__((aligned(64)));
    a[n & 31] = (unsigned char)n;
    return a[n & 31];
}

__attribute__((noinline)) unsigned pf_many(unsigned a, unsigned b, unsigned c,
                                           unsigned d, unsigned e, unsigned f,
                                           unsigned g, unsigned h, unsigned i,
                                           unsigned j)
{
    return a + b + c + d + e + f + g + h + i + j;
}

__attribute__((noinline)) unsigned pf_outgoing(unsigned n)
{
    return pf_many(n, 2, 3, 4, 5, 6, 7, 8, 9, 10) + 1;
}

__attribute__((noinline)) unsigned pf_dynamic(size_t n)
{
    if (!n)
    {
        return 0;
    }
    volatile unsigned char a[n];
    a[n - 1] = 7;
    return a[n - 1];
}

__attribute__((noinline)) unsigned pf_indirect(unsigned (*f)(unsigned),
                                               unsigned n)
{
    return f(n) + 1;
}

__attribute__((noinline)) unsigned pf_work(unsigned n)
{
    if (n & 1)
    {
        return pf_static(n) + 1;
    }
    return pf_bounded(n) + 2;
}

__attribute__((noinline)) unsigned pf_sequential(unsigned n)
{
    volatile unsigned work = 0;
    for (unsigned i = 0; i < n; ++i)
    {
        ++work;
    }
    for (unsigned j = 0; j < n; ++j)
    {
        ++work;
    }
    return work;
}

__attribute__((noinline)) unsigned pf_nested(unsigned n)
{
    volatile unsigned work = 0;
    for (unsigned i = 0; i < n; ++i)
    {
        for (unsigned j = 0; j < n; ++j)
        {
            ++work;
        }
    }
    return work;
}

int main(int argc, char **argv)
{
    (void)argv;
    sink = pf_work((unsigned)argc) + pf_dynamic(17) +
           pf_indirect(pf_static, 2) + pf_outgoing((unsigned)argc);
    unsigned a = pf_sequential(8), b = pf_nested(8);
    printf("CFG_WORK:%u:%u\n", a, b);
    return sink == 0 || a != 16 || b != 64;
}
