#ifndef AR_TEST_H
#define AR_TEST_H
#include "arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "AR_ASSERT %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(42);                                                        \
        }                                                                    \
    } while (0)
#endif
