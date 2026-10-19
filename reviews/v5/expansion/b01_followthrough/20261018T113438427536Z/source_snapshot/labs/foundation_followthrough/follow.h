#ifndef FF_FOLLOW_H
#define FF_FOLLOW_H
#include "depth.h"
/* Read CONTRACTS.md. These operations compose the completed FD provider. */
typedef struct { size_t begin, end; } FFInterval;
FDStatus ff_interval(size_t offset,size_t count,size_t width,size_t limit,FFInterval *out);
/* Callback context points to a distinct live FDValue; destination is the actual
 * newly allocated FDValue storage supplied by FDSlot. Copy its representation. */
int ff_copy_initialize(void *context,FDValue *destination);
FDStatus ff_encode(FDRecord record,unsigned char *out,size_t capacity);
FDStatus ff_decode(const unsigned char *wire,size_t length,FDRecord *out);
#endif
