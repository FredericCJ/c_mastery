#ifndef ND_TEST_H
#define ND_TEST_H
#include "numeric.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "ND_ASSERT %s:%d %s\n", __FILE__, __LINE__, #x);         \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
#endif
