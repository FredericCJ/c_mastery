#ifndef CASE
#error Define one isolated CASE
#endif
#if CASE == 1
#include <stdio.h>
#define A B
#define B 7
static int SELF = 5;
#define SELF SELF + 1
#define TWICE(x) ((x) + (x))

int main(void)
{
    printf("%d %d %d\n", A, SELF, TWICE(TWICE(3)));
    return 0;
}
#elif CASE == 2
#include <stdio.h>

static int F(int x)
{
    return x + 10;
}

#define F(x) ((x) + 1)
#define G F
#define H (F)

int main(void)
{
    int (*p)(int) = F, (*q)(int) = G;
    printf("%d %d %d %d %d %d\n", p(3), q(3), F(3), G(3), (F)(3), F /**/ (3));
    printf("%d\n", H(3));
    return 0;
}
#elif CASE == 3
#define PAIR 1, 7
#define ID(x) (x)
#define FORWARD(x) ID(x)
int result = FORWARD(PAIR);
#elif CASE == 4
struct lp_pair
{
    int a, b;
};

#define FIRST(x) ((x).a)

int main(void)
{
    return FIRST((struct lp_pair){3, 4});
}
#elif CASE == 5
#define CAT(a, b) a##b
CAT(x, +)
#elif CASE == 6
#define CAT(a, b) a##b
int CAT(item_, key)(int);
double item_key(double);
#elif CASE == 7
#include <stdio.h>
#define CAT(a, b) a##b
#define STR(x) #x
#define EXPAND_STR(x) STR(x)

int main(void)
{
    printf("%s|%s|%s\n", EXPAND_STR(CAT(, alpha)), EXPAND_STR(CAT(alpha, )),
           EXPAND_STR(CAT(, )));
    return 0;
}
#elif CASE == 8
#define CAT(a, b) a##b

static int CAT(item_, key)(int x)
{
    return x + 1;
}

int main(void)
{
    return item_key(2) == 3 ? 0 : 1;
}
#else
#error Unknown CASE
#endif
