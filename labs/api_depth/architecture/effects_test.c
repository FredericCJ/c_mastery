#define _POSIX_C_SOURCE 200809L
#include "effects.h"
#include "linux.h"
#include "test.h"
#include <unistd.h>
int main(void) {
  ArEffects state = {0};
  ArEffect effect;
  CHECK(ar_decide(&state, 3, &effect) == AR_OK && effect.id == 1 &&
        effect.value == 6);
  ArEffects saved = state;
  CHECK(ar_decide(&state, 4, &effect) == AR_STATE &&
        !memcmp(&state, &saved, sizeof state));
  char path[] = "/tmp/cmastery-effects-XXXXXX";
  int fd = mkstemp(path);
  CHECK(fd >= 0 && unlink(path) == 0);
  CHECK(ar_outbox(fd, effect.id, effect.value, 1) == AR_PENDING);
  CHECK(ar_feedback(&state, AR_PENDING) == AR_PENDING && state.pending);
  ArEffect retry;
  CHECK(ar_retry(&state, &retry) == AR_OK && retry.id == effect.id &&
        retry.value == effect.value);
  CHECK(ar_outbox(fd, retry.id, retry.value, 0) == AR_OK);
  CHECK(ar_feedback(&state, AR_OK) == AR_OK && !state.pending);
  CHECK(ar_retry(&state, &retry) == AR_STATE);
  CHECK(ar_decide(&state, 4, &effect) == AR_OK && effect.id == 2 &&
        effect.value == 8);
  CHECK(ar_outbox(fd, effect.id, effect.value, 0) == AR_STATE);
  CHECK(ar_feedback(&state, AR_FAILED) == AR_PENDING && state.pending);
  CHECK(close(fd) == 0);
  state = (ArEffects){.next = UINT32_MAX};
  CHECK(ar_decide(&state, 1, &effect) == AR_STATE);
  puts("PASS pure effect decision,external commit,lost acknowledgment,same-ID "
       "retry,nonrollback boundary");
}
