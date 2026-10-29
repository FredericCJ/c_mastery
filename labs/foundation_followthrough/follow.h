#ifndef FF_FOLLOW_H
#define FF_FOLLOW_H
#include "depth.h"

/* Read CONTRACTS.md. These operations compose the completed FD provider. */
struct ff_interval
{
    size_t begin, end;
};
enum fd_status ff_interval(size_t offset, size_t count, size_t width,
                           size_t limit, struct ff_interval *out);
/* Callback context points to a distinct live FDValue; destination is the actual
 * newly allocated FDValue storage supplied by FDSlot. Copy its representation. */
int ff_copy_initialize(void *context, struct fd_value *destination);
enum fd_status ff_encode(struct fd_record record, unsigned char *out,
                         size_t capacity);
enum fd_status ff_decode(const unsigned char *wire, size_t length,
                         struct fd_record *out);
#endif
