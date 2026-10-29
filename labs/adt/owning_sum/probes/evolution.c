#include <stdio.h>
#ifndef ADDED
#define ADDED 0
#endif
#ifndef ELIMINATOR
#define ELIMINATOR 0
#endif
enum version_tag
{
    OLD_EMPTY,
    OLD_NUMBER,
    OLD_PAIR
#if ADDED
        ,
    NEW_TEXT
#endif
};

static unsigned measure(enum version_tag tag)
{
    switch (tag)
    {
        case OLD_EMPTY:
            return 0;
        case OLD_NUMBER:
            return 1;
        case OLD_PAIR:
            return 2;
#if ADDED && ELIMINATOR == 2
        case NEW_TEXT:
            return 3;
#endif
#if ELIMINATOR == 1
        default:
            return 0;
#endif
    }
    return 99;
}

int main(void)
{
#if ADDED
    unsigned observed = measure(NEW_TEXT);
#else
    unsigned observed = measure(OLD_PAIR);
#endif
    printf("tag-size%zu value%u\n", sizeof(enum version_tag), observed);
#if ADDED
    return observed == (ELIMINATOR == 1 ? 0u : 3u) ? 0 : 1;
#else
    return observed == 2 ? 0 : 1;
#endif
}
