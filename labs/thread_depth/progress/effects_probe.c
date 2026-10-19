#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
/* Compile and inspect only: explicit whole-path dependencies, not an ISR. */
int tp_effect_path(pthread_mutex_t *lock, size_t n, int fd) {
  unsigned char *bytes = malloc(n);
  if (!bytes)
    return -1;
  int e = pthread_mutex_lock(lock);
  if (!e) {
    if (n)
      bytes[0] = 42;
    if (n && write(fd, bytes, 1) != 1)
      e = -1;
    int unlock = pthread_mutex_unlock(lock);
    if (unlock)
      e = unlock;
  }
  free(bytes);
  return e;
}
