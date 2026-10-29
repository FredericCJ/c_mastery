#include <stddef.h>
#include <stdio.h>
#ifndef CASE
#define CASE 0
#endif
#if CASE == 1
int *pointer_constraint = 1;
#elif CASE == 2
struct ff_packet
{
    const unsigned char *p;
    size_t n;
};

int packet_at(struct ff_packet, size_t);

int bad(void)
{
    return packet_at(3, 0);
}
#else
static int x, y, state;

static int f(void)
{
    return ++x;
}

static int g(void)
{
    return y *= 2;
}

static int shared_f(void)
{
    return state++;
}

static int shared_g(void)
{
    return state *= 2;
}

static int sum(int a, int b)
{
    return a + b;
}

int main(void)
{
    x = 1;
    y = 2;
    int distinct = sum(f(), g());
    if (distinct != 6 || x != 2 || y != 4)
    {
        return 1;
    }
    state = 1;
    int shared = sum(shared_f(), shared_g());
    if (!((shared == 5 && state == 4) || (shared == 4 && state == 3)))
    {
        return 1;
    }
    printf("distinct=%d,%d,%d shared=%d,%d\n", distinct, x, y, shared, state);
    return 0;
}
#endif
