/*
 * Checked decimal conversion for command-line and environment inputs.
 *
 * Accepted grammar: one or more ASCII digits '0'..'9' and nothing else.
 * Rejected: NULL or empty text (CM_PARSE_EMPTY); any sign, whitespace, base
 * prefix, separator or trailing character (CM_PARSE_SYNTAX); a value above
 * UINTMAX_MAX or outside the caller's [min, max] (CM_PARSE_RANGE). Leading
 * zeros are accepted. Only base 10 is supported.
 *
 * The range check happens on the uintmax_t value before any narrowing, so a
 * value that would wrap into the destination type (for example 4294967297
 * stored into a 32-bit unsigned) is rejected instead of becoming a smaller,
 * valid-looking request (SEI CERT C ERR34-C, INT31-C).
 *
 * Hosted environments only: it relies on <errno.h> and strtoumax from the C
 * library, which the course's freestanding targets deliberately do not link.
 *
 * On success the result is stored in *value; on any failure *value is left
 * unchanged. The functions are reentrant and do not modify errno's meaning for
 * the caller beyond what strtoumax sets.
 */
#ifndef CM_PARSE_H
#define CM_PARSE_H

#include <stddef.h>
#include <stdint.h>

enum cm_parse_status
{
    CM_PARSE_OK = 0,
    CM_PARSE_EMPTY,
    CM_PARSE_SYNTAX,
    CM_PARSE_RANGE
};

enum cm_parse_status cm_parse_umax(const char *text, uintmax_t min,
                                   uintmax_t max, uintmax_t *value);
enum cm_parse_status cm_parse_unsigned(const char *text, unsigned min,
                                       unsigned max, unsigned *value);
enum cm_parse_status cm_parse_size(const char *text, size_t min, size_t max,
                                   size_t *value);
const char *cm_parse_status_name(enum cm_parse_status status);

#endif /* CM_PARSE_H */
