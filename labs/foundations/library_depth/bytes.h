#ifndef LIBRARY_BYTES_H
#define LIBRARY_BYTES_H
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>

enum lb_status
{
    LB_OK,
    LB_ARGUMENT,
    LB_RANGE,
    LB_FULL,
    LB_UNTERMINATED,
    LB_NOT_IMPLEMENTED
};
enum lb_status lb_move(unsigned char *, size_t, size_t, size_t, size_t);
/* Different live arrays, disjoint extents proved by caller; no pointer
 * ordering. */
enum lb_status lb_copy(unsigned char *, size_t, const unsigned char *, size_t,
                       size_t);
enum lb_status lb_strlen(const unsigned char *, size_t, size_t *);
/* Exact n-byte payload; printable ASCII except backslash is copied, other
   octets become four ASCII bytes backslash,x,uppercase hexadecimal digits. */
enum lb_status lb_display(const unsigned char *, size_t, char *, size_t,
                          size_t *);
/* Big-endian two-octet length then payload, consumed transactionally. */
enum lb_status lb_field_display(struct f_cursor *, char *, size_t, size_t *);
/* Reject length>width. Copy exact bytes then pad to width, without a C-string
 * promise. */
enum lb_status lb_record(const unsigned char *, size_t, unsigned char *,
                         size_t);
#endif
