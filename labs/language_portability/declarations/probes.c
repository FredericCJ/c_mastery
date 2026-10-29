/* Each selection is a separate translation, not one feature omnibus. */
#include <stddef.h>
#include <limits.h>
#ifndef LD_PROBE
#define LD_PROBE 1
#endif
#ifndef LD_VM_EXPECTATION
#define LD_VM_EXPECTATION 1
#endif
#if LD_PROBE == 1
int main(void)
{
    int *p = nullptr;
    nullptr_t n = nullptr;
    return p != n || (bool)n;
}
#elif LD_PROBE == 2
int main(void)
{
    const _Atomic int a = 3;
    typeof(a) b = 4;
    typeof_unqual(a) c = 5;
    static_assert(_Generic(&b, const _Atomic int *: 1, default: 0));
    static_assert(_Generic(&c, int *: 1, default: 0));
    c++;
    return b != 4 || c != 6;
}
#elif LD_PROBE == 3
int main(void)
{
    const int a[3] = {1, 2, 3};
    typeof(a) b = {4, 5, 6};
    auto p = a;
    auto x = a[0];
    static_assert(_Generic(&b, const int (*)[3]: 1, default: 0));
    static_assert(_Generic(&p, const int **: 1, default: 0));
    static_assert(_Generic(&x, int *: 1, default: 0));
    x++;
    return b[2] != 6 || p[2] != 3 || x != 2;
}
#elif LD_PROBE == 4
int main(void)
{
    int n = 3, calls = 0;
    int a[n];
    int (*p)[n] = &a;
    typeof((calls++, p)) q = p; /* Pointer to VLA remains variably modified. */
    typeof(calls++) c = 0; /* Ordinary int operand is not evaluated. */
    return calls != LD_VM_EXPECTATION || sizeof *q != sizeof a || c != 0;
}
#elif LD_PROBE == 5
int main(void)
{
    unsigned _BitInt(1) u = 1;
    _BitInt(2) s = -2;
    return u != 1 || s != -2;
}
#elif LD_PROBE == 6
_BitInt(1) invalid;

int main(void)
{
    return 0;
}
#elif LD_PROBE == 7
unsigned _BitInt(BITINT_MAXWIDTH + 1) invalid;

int main(void)
{
    return 0;
}
#elif LD_PROBE == 8
enum lp_wide
{
    WIDE_VALUE = 4294967296ULL
};

enum Fixed : unsigned long long
{
    FIXED_VALUE = 4294967296ULL
};

int main(void)
{
    static_assert(WIDE_VALUE == 4294967296ULL);
    static_assert(FIXED_VALUE == 4294967296ULL);
    return 0;
}
#elif LD_PROBE == 9
enum Invalid : unsigned _BitInt(13)
{
    VALUE = 1
};

int main(void)
{
    return 0;
}
#elif LD_PROBE == 10
constexpr unsigned count = 7;
constexpr int *empty = nullptr;
static_assert(count * count == 49);

int main(void)
{
    int a[count];
    return sizeof a / sizeof a[0] != 7 || empty != nullptr;
}
#elif LD_PROBE == 11
constexpr int square(int n)
{
    return n * n;
}

int main(void)
{
    return square(2);
}
#elif LD_PROBE == 12
int value;
constexpr int *bad = &value;

int main(void)
{
    return 0;
}
#elif LD_PROBE == 13
int f(int n)
{
    return n * n;
}

_Static_assert(f(7) == 49,
               "ordinary C call is not an integer constant expression");

int main(void)
{
    return 0;
}
#elif LD_PROBE == 14
int main(void)
{
    int n = 3, calls = 0;
    int a[n];
    int (*p)[n] = &a;
    typeof((calls++,
            *p)) b; /* Comma array conversion changes the operand type. */
    static_assert(_Generic(&b, int **: 1, default: 0));
    return calls != 0;
}
#else
#error Unknown probe
#endif
