#ifndef LIBRARY_SCANNER_H
#define LIBRARY_SCANNER_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
enum ls_status {
  LS_OK,
  LS_ARGUMENT,
  LS_SYNTAX,
  LS_RANGE,
  LS_LONG,
  LS_END,
  LS_IO,
  LS_NOT_IMPLEMENTED
};
struct ls_record {
  int32_t count;
  size_t length;
  unsigned char name[33];
};
enum ls_status ls_record_parse(const unsigned char *, size_t, int,
                               struct ls_record *);
/* Finite owned/borrowed byte stream. Consume one complete line, excluding
   newline. cap1..129. Failure leaves output/length; overlong lines are fully
   discarded. */
enum ls_status ls_line(FILE *, unsigned char *, size_t, size_t *);
#endif
