#ifndef HF_FLOW_H
#define HF_FLOW_H
#include "com_http.h"
#include <stddef.h>
#include <stdint.h>
typedef struct { int fd; uint32_t generation; unsigned live, pending, retired; } HFSlot;
typedef struct { HFSlot slot[4]; } HFPool;
typedef struct { unsigned index; uint32_t generation; } HFHandle;
/* A queued callback pins its slot storage. Closing invalidates dispatch but
   storage cannot be recycled until every successful pin is completed. */
int hf_admit(HFPool *, int fd, unsigned flags, HFHandle *);
int hf_pin(HFPool *, HFHandle);
int hf_callback(HFPool *, HFHandle, int *dispatch_fd);
int hf_close(HFPool *, HFHandle, int *close_fd);
typedef struct {int (*accept)(void *);int (*flags)(void *,int);void (*close)(void *,int);void *context;} HFAccept;
/* accept callback: descriptor>=0; -1 interrupted; -2 would-block; -3 fatal.
   At most four attempts. Flags and admission precede publication. */
int hf_accept_batch(HFPool *, HFAccept, HFHandle handles[4], unsigned *count);
/* Date NULL means unavailable and omits the field. Error representation NULL
   selects reason+LF. Explicit bytes permit a UTF-8 changed representation. */
int hf_response(const HttpRequest *, const char *date, int close,
                const void *error, size_t error_length, void *, size_t, size_t *);
typedef struct { unsigned char bytes[1600]; size_t length, offset; uint64_t started; unsigned request, interim; } HFOutput;
typedef struct {
  HttpParser parser;
  HFOutput output[2];
  unsigned capacity, head, count, requests, closing, draining, aborted;
  uint64_t started, progress, drain_started;
} HFFlow;
int hf_init(HFFlow *, unsigned capacity, const char *, uint64_t);
int hf_feed(HFFlow *, const void *, size_t, size_t *, uint64_t, const char *, const char *date);
int hf_eof(HFFlow *, uint64_t, const char *date);
/* recv result adapter: -1 EINTR/-2 EAGAIN preserve state;0 is realEOF;
   other negative values abort. Positive bytes are separately retained/fed. */
int hf_read_event(HFFlow *, int result, uint64_t, const char *date);
int hf_write(HFFlow *, HttpWrite, void *, uint64_t);
/* 0 wait;1 begin write shutdown;2 close after drain;3 output abort;
   request expiry enqueues408 and returns0. Monotonic regression aborts. */
int hf_tick(HFFlow *, uint64_t, const char *date);
#endif
