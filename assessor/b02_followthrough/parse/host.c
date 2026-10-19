#include "parse.h"
#include <errno.h>
#include <stdlib.h>
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
_Static_assert('0' == 48 && '9' == 57 && 'A' == 65 && 'Z' == 90 && '-' == 45,
               "Hosted ASCII comparison profile");
int bf_parse_libc(const unsigned char *p, size_t n, unsigned base,
                  int32_t *out) {
  if (!out || (!p && n) || base < 2 || base > 36)
    return B2_INVALID;
  if (!n)
    return B2_INVALID;
  size_t i = p[0] == 45;
  if (i == n || (p[i] == 48 && (n - i != 1 || i)))
    return B2_INVALID;
  for (size_t j = i; j < n; j++) {
    unsigned d = p[j] >= 48 && p[j] <= 57   ? p[j] - 48u
                 : p[j] >= 65 && p[j] <= 90 ? p[j] - 65u + 10u
                                            : 36u;
    if (d >= base)
      return B2_INVALID;
  }
  if (n == SIZE_MAX)
    return B2_FULL;
  char *text = malloc(n + 1);
  if (!text)
    return B2_FULL;
  for (size_t j = 0; j < n; j++)
    text[j] = (char)p[j];
  text[n] = 0;
  char *end;
  if (BF_MUTANT != 4)
    errno = 0;
  long value = strtol(text, &end, (int)base);
  int r = end != text + n ? B2_INVALID
          : errno == ERANGE || value < INT32_MIN || value > INT32_MAX ? B2_RANGE
                                                                      : B2_OK;
  free(text);
  if (r == B2_OK)
    *out = (int32_t)value;
  return r;
}
