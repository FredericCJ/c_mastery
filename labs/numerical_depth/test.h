#ifndef ND_TEST_H
#define ND_TEST_H
#include "numeric.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Prints "ND_ASSERT file:line expression" to stderr and exits with status 1.
 * The campaign accepts a rejection only when this marker appears.
 */
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "ND_ASSERT %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)
#endif
