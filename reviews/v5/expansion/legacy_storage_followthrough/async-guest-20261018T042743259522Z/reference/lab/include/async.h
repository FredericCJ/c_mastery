#ifndef STORAGE_FOLLOW_ASYNC_H
#define STORAGE_FOLLOW_ASYNC_H
#include <stdint.h>
enum {
  SA_OK = 0,
  SA_INVALID,
  SA_BUSY,
  SA_IO,
  SA_READY,
  SA_DRAINED,
  SA_UNKNOWN,
  SA_UNIMPLEMENTED
};
enum { SA_EMPTY = 0, SA_OWNED, SA_COMPLETED, SA_QUARANTINED };
typedef struct {
  void *context;
  int64_t (*submit_read)(void *, uint32_t, void *);
  int64_t (*wait)(void *, uint64_t);
} SaPort;
typedef struct {
  SaPort port;
  void *buffer;
  uint64_t ticket;
  unsigned state, abandoned;
} SaRead;
/* Zero-initialized exclusive control; live writable4096-byte buffer and port
 * context through successful wait and explicit release. Submit positive means
 * accepted; nonpositive means no queued request. Wait0 proves completion;
 * any other result does NOT prove memory quiescence. No callback reentry. */
int sa_start(SaRead *, SaPort, uint32_t, void *);
int sa_abandon(SaRead *);
int sa_finish(SaRead *);
int sa_release(SaRead *, void **);
#endif
