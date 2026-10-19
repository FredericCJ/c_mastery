#ifndef CMASTERY_STREAM_VARIANTS_H
#define CMASTERY_STREAM_VARIANTS_H
#include "stream.h"

enum { SV_PROGRESS, SV_WAIT, SV_RETRY, SV_ERROR };
typedef struct {
  size_t accepted;
  unsigned disposition;
} SVWriteResult;
typedef SVWriteResult (*SVWrite)(void *, const unsigned char *, size_t);
typedef struct {
  const unsigned char *data;
  size_t length;
} SVSpan;
typedef SVWriteResult (*SVWriteV)(void *, const SVSpan *, size_t);
typedef struct {
  SFStream frame;
  SFHandle base;
  int status;
} SVStream;
typedef struct {
  SVStream *owner;
  uint32_t generation;
} SVHandle;

void sv_init(SVStream *);
int sv_start(SVStream *, const unsigned char *, size_t, SVHandle *);
int sv_pump(SVHandle, SVWrite, void *, unsigned, size_t *);
int sv_pumpv(SVHandle, SVWriteV, void *, unsigned, size_t *);
int sv_cancel(SVHandle);

typedef struct {
  SVStream slots[2];
  uint64_t tickets[2], next_ticket;
  unsigned active[2], tainted;
} SVQueue;
typedef struct {
  SVHandle handle;
  size_t accepted;
  int status;
} SVEvent;

void svq_init(SVQueue *);
int svq_submit(SVQueue *, const unsigned char *, size_t, SVHandle *);
int svq_pump(SVQueue *, SVWrite, void *, unsigned, SVEvent *);
int svq_cancel(SVQueue *, SVHandle, SVEvent *);
int svq_reset_transport(SVQueue *, unsigned verified_quiescent);
#endif
