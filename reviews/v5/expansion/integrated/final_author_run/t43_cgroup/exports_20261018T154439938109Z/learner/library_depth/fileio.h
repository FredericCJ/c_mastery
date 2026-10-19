#ifndef LIBRARY_FILEIO_H
#define LIBRARY_FILEIO_H
#include "foundation.h"
#include <stddef.h>
enum li_status {
  LI_PENDING,
  LI_DONE,
  LI_READ_ERROR,
  LI_WRITE_ERROR,
  LI_PROTOCOL,
  LI_ARGUMENT,
  LI_CLOSED,
  LI_CLOSE_ERROR,
  LI_FLUSH_ERROR,
  LI_NOT_IMPLEMENTED
};
enum li_read_status { LI_MORE, LI_EOF, LI_ERROR, LI_WAIT };
struct li_read_result {
  size_t count;
  enum li_read_status status;
};
typedef struct li_read_result (*LIRead)(void *, unsigned char *, size_t);
struct li_copy {
  unsigned char bytes[2];
  size_t length, offset, read_total, written_total;
  enum li_read_status after_read;
  enum li_status terminal;
};
void li_init(struct li_copy *);
enum li_status li_pump(struct li_copy *, LIRead, void *, FWriter, void *,
                       unsigned);
struct li_owner {
  void *handle;
  int owned, output;
};
/* Owned: close exactly once, consumes ownership even on failure.
   Borrowed output: flush once, never close. Borrowed input: neither. */
enum li_status li_release(struct li_owner *, int (*flush)(void *),
                          int (*close)(void *));
#endif
