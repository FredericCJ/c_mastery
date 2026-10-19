#include "ipc.h"
#include <errno.h>
int ei_exchange(EiIo o, const unsigned char request[8],
                unsigned char response[4], struct timespec deadline,
                EiProgress *progress) {
  (void)o;
  (void)request;
  (void)response;
  (void)deadline;
  (void)progress;
  errno = ENOSYS;
  return -1;
}
