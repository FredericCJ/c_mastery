#include "service_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "COMPOSITION_CHECK:%d:%s\n", __LINE__, #x);              \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int validate(void *ctx, void *out, size_t n) {
  (void)out;
  (void)n;
  return *(int *)ctx;
}
int main(void) {
  /* Original wire request: one-octet operation ID, one-octet positive delta.
   * Bounds below are the complete parser for this intentionally tiny profile.
   */
  static const unsigned char request[2] = {1, 7};
  struct sd_entry entry = {"request", request, 2};
  struct sd_files fs;
  struct sd_handle fd;
  struct sd_channel q;
  struct sd_journal j = {0};
  struct sd_cap cap = {1, 7};
  unsigned char wire[2] = {99, 99};
  size_t n = 99;
  int fail = 1;
  int32_t answer = 99;
  struct sd_receipt receipt;
  CHECK(sd_files_init(&fs, &entry, 1) == SD_OK);
  CHECK(sd_open(&fs, "request", &fd) == SD_OK);
  CHECK(sd_read(&fs, fd, wire, 2, 2, &n, validate, &fail) == SD_COPY &&
        n == 99);
  fail = 0;
  CHECK(sd_read(&fs, fd, wire, 2, 2, &n, validate, &fail) == SD_OK && n == 2);
  CHECK(!memcmp(wire, request, 2));
  CHECK(sd_channel_init(&q) == SD_OK && sd_send(&q, cap, wire, n) == SD_OK);
  wire[0] = 99;
  fail = 1;
  CHECK(sd_receive(&q, cap, wire, 2, &n, validate, &fail) == SD_COPY);
  CHECK(sd_recover(&j, &receipt) == SD_OK && receipt.last == 0);
  fail = 0;
  CHECK(sd_receive(&q, cap, wire, 2, &n, validate, &fail) == SD_OK);
  CHECK(n == 2 && wire[0] != 0 && wire[1] <= 127);
  CHECK(sd_apply(&j, wire[0], wire[1], 4, &answer) == SD_UNCERTAIN &&
        answer == 99);
  /* Simulated reset loses only volatile buffers and reply. */
  memset(j.pending, 0, sizeof j.pending);
  CHECK(sd_recover(&j, &receipt) == SD_OK && receipt.last == 1 &&
        receipt.value == 7);
  CHECK(sd_seek(&fs, fd, 0, 0) == SD_OK);
  CHECK(sd_read(&fs, fd, wire, 2, 2, &n, validate, &fail) == SD_OK);
  CHECK(sd_send(&q, cap, wire, n) == SD_OK);
  CHECK(sd_receive(&q, cap, wire, 2, &n, validate, &fail) == SD_OK);
  CHECK(sd_apply(&j, wire[0], wire[1], 0, &answer) == SD_OK && answer == 7);
  CHECK(sd_apply(&j, wire[0], wire[1] + 1, 0, &answer) == SD_CONFLICT &&
        answer == 7);
  CHECK(sd_close(&fs, fd) == SD_OK);
  CHECK(sd_channel_close(&q, cap) == SD_OK);
  puts("service composition: failed copy preserves ownership; reset/retry "
       "preserves single durable effect");
  return 0;
}
