#ifndef LIBRARY_SORTING_H
#define LIBRARY_SORTING_H
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>
struct lr_record {
  double key;
  unsigned identity;
};
enum lr_status { LR_OK, LR_ARGUMENT, LR_RETRY, LR_SOURCE, LR_NOT_IMPLEMENTED };
int lr_compare_i32(const void *, const void *);
int lr_compare_record(const void *, const void *);
enum lr_status lr_sort(int32_t *, size_t, FWork *);
/* Draw returns0..7, or a negative source failure. At most budget calls. */
typedef int (*lr_draw)(void *);
enum lr_status lr_three(lr_draw, void *, unsigned budget, unsigned *out);
/* Byte source: positive prefix<=capacity,-2 retry,zero/other negative failure.
   Exactly32 bytes are published only after a complete successful acquisition.
 */
typedef ptrdiff_t (*lr_bytes)(void *, unsigned char *, size_t);
enum lr_status lr_entropy(lr_bytes, void *, unsigned budget,
                          unsigned char out[32]);
#endif
