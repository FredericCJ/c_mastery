#ifndef LIBRARY_CONVERSION_H
#define LIBRARY_CONVERSION_H
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
enum lc_status {
  LC_OK,
  LC_SYNTAX,
  LC_RANGE,
  LC_ARGUMENT,
  LC_TOO_LONG,
  LC_BACKEND,
  LC_NOT_IMPLEMENTED
};
enum lc_status lc_i8(const unsigned char *, size_t, int, int32_t *);
enum lc_status lc_unsigned(const unsigned char *, size_t, int, uintmax_t,
                           uintmax_t *);
struct lc_float {
  double value;
  size_t consumed;
  int range;
};
typedef struct lc_float (*lc_converter)(void *, const char *);
/* Canonical decimal rational: 0 OR -?[1-9](.[0-9]*[1-9])?e(0|-?[1-9][0-9]*).
   This normalizes syntax/rational notation, not binary floating-point values.
 */
enum lc_status lc_wire_float(const unsigned char *, size_t, lc_converter,
                             void *, double *);
#endif
