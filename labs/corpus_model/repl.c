#include "model.h"
#include <stdio.h>
static size_t output(void *context, const char *s, size_t n) {
  return fwrite(s, 1, n, context);
}
static int flush(struct cm_session *s) {
  int rc;
  do {
    rc = cm_drain(s, output, stdout);
  } while (rc == CM_PENDING);
  return rc || fflush(stdout) == EOF;
}
int main(void) {
  struct cm_session session;
  cm_session_init(&session);
  while (!session.closed) {
    int c = fgetc(stdin);
    if (c == EOF) {
      if (ferror(stdin))
        return 2;
      (void)cm_finish(&session);
      return flush(&session) ? 3 : 0;
    }
    unsigned char byte = (unsigned char)c;
    size_t used;
    int rc = cm_feed(&session, &byte, 1, &used);
    if (rc != CM_OK && rc != CM_PENDING)
      return 4;
    if (flush(&session))
      return 3;
  }
  return 0;
}
