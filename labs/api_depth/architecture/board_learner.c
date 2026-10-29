#include "board_input.h"
struct ar_result ar_f439_read(struct fh_io io, unsigned char *bytes, size_t capacity) {
  (void)io;
  (void)bytes;
  (void)capacity;
  return (struct ar_result){0, AR_UNIMPLEMENTED};
}
