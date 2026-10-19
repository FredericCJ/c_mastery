#ifndef LIBRARY_UNICODE_H
#define LIBRARY_UNICODE_H
#include <stddef.h>
#include <stdint.h>
enum lu_status {
  LU_OK,
  LU_INCOMPLETE,
  LU_INVALID,
  LU_FULL,
  LU_ARGUMENT,
  LU_NOT_IMPLEMENTED
};
enum lu_policy { LU_STRICT, LU_REPLACE };
/* Only initialize to all zero, then mutate through this API. Four pending bytes
   suffice, but a live retained prefix always contains one to three bytes. */
struct lu_state {
  unsigned char bytes[4];
  unsigned used;
};
struct lu_result {
  size_t consumed, produced;
};
enum lu_status lu_decode(struct lu_state *, const unsigned char *, size_t,
                         uint32_t *, size_t, enum lu_policy,
                         struct lu_result *);
enum lu_status lu_finish(struct lu_state *, uint32_t *, size_t, enum lu_policy,
                         struct lu_result *);
enum lu_status lu_encode(uint32_t, unsigned char *, size_t, size_t *);
enum lu_status lu_utf16(uint32_t, uint16_t *, size_t, size_t *);
#endif
