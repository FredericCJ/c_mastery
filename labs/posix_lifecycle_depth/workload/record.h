#ifndef PW_RECORD_H
#define PW_RECORD_H
#include <stddef.h>
/* Borrowed seekable fd; externally fixed extent <=1MiB, record <=64 bytes.
 * Output is unchanged on validation, short-EOF or syscall failure. */
int pw_read_record(int fd,size_t logical,size_t offset,size_t length,void *out,size_t capacity);
#endif
