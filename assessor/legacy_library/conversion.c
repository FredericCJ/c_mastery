#include "conversion.h"
#include "foundation.h"
#include <errno.h>
#include <math.h>
#include <stdlib.h>
static int digit(unsigned char c) { return c >= '0' && c <= '9'; }
static int hex(unsigned char c) {
  return digit(c)               ? c - '0'
         : c >= 'a' && c <= 'f' ? c - 'a' + 10
         : c >= 'A' && c <= 'F' ? c - 'A' + 10
                                : -1;
}
enum lc_status lc_i8(const unsigned char *s, size_t n, int plus, int32_t *out) {
  if (!out || (!s && n) || (plus != 0 && plus != 1))
    return LC_ARGUMENT;
  if (!n)
    return LC_SYNTAX;
  if (n > 128)
    return LC_TOO_LONG;
  size_t start = (s[0] == '-' || (plus && s[0] == '+')) ? 1 : 0;
  if (start == n)
    return LC_SYNTAX;
  for (size_t i = start; i < n; ++i)
    if (!digit(s[i]))
      return LC_SYNTAX;
  int32_t value;
  FStatus status = f_parse_i32(s + (s[0] == '+'), n - (s[0] == '+'), &value);
  if (status == F_RANGE)
    return LC_RANGE;
  if (status != F_OK)
    return LC_SYNTAX;
  if (value < -128 || value > 127)
    return LC_RANGE;
  *out = value;
  return LC_OK;
}
enum lc_status lc_unsigned(const unsigned char *s, size_t n, int prefixes,
                           uintmax_t limit, uintmax_t *out) {
  if (!out || (!s && n) || (prefixes != 0 && prefixes != 1))
    return LC_ARGUMENT;
  if (!n)
    return LC_SYNTAX;
  if (n > 128)
    return LC_TOO_LONG;
  size_t i = s[0] == '+';
  if (i == n)
    return LC_SYNTAX;
  unsigned base = 10;
  if (prefixes && s[i] == '0') {
    base = 8;
    if (n - i >= 2 && (s[i + 1] == 'x' || s[i + 1] == 'X')) {
      base = 16;
      i += 2;
      if (i == n)
        return LC_SYNTAX;
    }
  }
  for (; i < n; ++i) {
    int d = hex(s[i]);
    if (d < 0 || (unsigned)d >= base)
      return LC_SYNTAX;
  }
  char token[129];
  for (i = 0; i < n; ++i)
    token[i] = (char)s[i];
  token[n] = 0;
  char *end;
#ifndef LC_MUTANT_ERRNO
  errno = 0;
#endif
  uintmax_t value = strtoumax(token, &end, prefixes ? 0 : 10);
  if (end != token + n)
    return LC_BACKEND;
  if (errno == ERANGE)
    return LC_RANGE;
#ifndef LC_MUTANT_NARROW
  if (value > limit)
    return LC_RANGE;
#endif
  *out = value;
  return LC_OK;
}
static int canonical(const unsigned char *s, size_t n) {
  if (n == 1 && s[0] == '0')
    return 1;
  size_t i = s[0] == '-';
  if (i == n || s[i] < '1' || s[i] > '9')
    return 0;
  ++i;
  if (i < n && s[i] == '.') {
    size_t first = ++i;
    while (i < n && digit(s[i]))
      ++i;
    if (i == first || s[i - 1] == '0')
      return 0;
  }
  if (i == n || s[i++] != 'e' || i == n)
    return 0;
  if (s[i] == '0')
    return i + 1 == n;
  if (s[i] == '-')
    ++i;
  if (i == n || s[i] < '1' || s[i] > '9')
    return 0;
  while (i < n && digit(s[i]))
    ++i;
  return i == n;
}
enum lc_status lc_wire_float(const unsigned char *s, size_t n,
                             lc_converter convert, void *ctx, double *out) {
  if (!out || !convert || (!s && n))
    return LC_ARGUMENT;
  if (!n)
    return LC_SYNTAX;
  if (n > 128)
    return LC_TOO_LONG;
#ifndef LC_MUTANT_GRAMMAR
  if (!canonical(s, n))
    return LC_SYNTAX;
#endif
  char token[129];
  for (size_t i = 0; i < n; ++i)
    token[i] = (char)s[i];
  token[n] = 0;
  struct lc_float value = convert(ctx, token);
  if (value.consumed != n)
    return LC_BACKEND;
  if (value.range || !isfinite(value.value))
    return LC_RANGE;
  *out = value.value;
  return LC_OK;
}
