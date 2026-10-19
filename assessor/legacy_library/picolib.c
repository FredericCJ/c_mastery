#include "picolib.h"
#include <stdio.h>
static unsigned char bytes[4];
static size_t used, position;
static int send(char byte, FILE *f) {
  (void)f;
  if (used == sizeof bytes) {
#ifdef LP_MUTANT_DROP
    return (unsigned char)byte;
#else
    return EOF;
#endif
  }
  bytes[used++] = (unsigned char)byte;
  return (unsigned char)byte;
}
static int receive(FILE *f) {
  (void)f;
  static const char input[] = "abc";
  return position < 3 ? (unsigned char)input[position++] : EOF;
}
static FILE console = FDEV_SETUP_STREAM(send, receive, NULL, _FDEV_SETUP_RW);
FILE *const stdin = &console;
FILE *const stdout = &console;
FILE *const stderr = &console;
int lp_reset(void) {
  used = position = 0;
  clearerr(&console);
  return 0;
}
size_t lp_output(unsigned char out[4]) {
  for (size_t i = 0; i < used; ++i)
    out[i] = bytes[i];
  return used;
}
