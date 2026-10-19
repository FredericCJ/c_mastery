#ifndef LIBRARY_HEADERS_H
#define LIBRARY_HEADERS_H
#include <stddef.h>
#include <stdint.h>
enum lh_status { LH_OK, LH_ARGUMENT, LH_SYNTAX, LH_RANGE, LH_NOT_IMPLEMENTED };
/* Counted ASCII decimal, nonempty, no signs/space, failure preserves *out. */
enum lh_status lh_size(const unsigned char *bytes, size_t n, size_t *out);
/* All four capabilities are observations, not a conformance certificate. */
int lh_c11_threads(int header, long edition, int no_threads, int link_pass);
#endif
