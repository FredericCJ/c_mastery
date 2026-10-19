#ifndef CMASTERY_SYNC_DEPTH_MAILBOX_H
#define CMASTERY_SYNC_DEPTH_MAILBOX_H
#include "cl.h"
#include "cmx.h"
enum { CDMB_UNKNOWN = CMX_UNIMPLEMENTED + 1, CDMB_STALE };
enum { CDMB_IDLE, CDMB_ACTIVE, CDMB_TERMINAL };
enum { CDMB_GENERATION_MAX = 65535 };
typedef struct {
  uint32_t session, generation;
  unsigned slot;
} CDMBIdentity;
typedef struct {
  CDMBIdentity id;
  unsigned char bytes[4];
} CDMBRequest;
typedef struct {
  uint32_t generation;
  unsigned phase, send_pending, progress_pending, delivered, received,
      has_reply;
  const unsigned char *borrowed;
  unsigned char request[4], reply[4];
  cmx_deadline deadline;
  int result;
  cmx_sem progress;
} CDMBClient;
typedef struct {
  cmx_kernel *kernel;
  cmx_queue queue;
  CDMBClient client[2];
  CLAck receipts[2];
  uint32_t session;
  unsigned capacity, receive_pending;
} CDMailbox;
int cdmb_init(CDMailbox *, cmx_kernel *, unsigned capacity, uint32_t session);
int cdmb_begin(CDMailbox *, unsigned slot, const unsigned char request[4],
               cmx_deadline, CDMBIdentity *);
int cdmb_delivery(CDMailbox *, unsigned slot);
int cdmb_receive(CDMailbox *, CDMBRequest *, unsigned capacity, cmx_deadline);
int cdmb_reply(CDMailbox *, CDMBIdentity, const unsigned char reply[4]);
int cdmb_poll(CDMailbox *, unsigned slot, unsigned char *reply,
              unsigned capacity);
int cdmb_cancel(CDMailbox *, unsigned slot);
int cdmb_restart(CDMailbox *);
int cdmb_effect(CDMailbox *, CDMBIdentity, uint32_t delta, uint32_t *result);
int cdmb_acknowledge(CDMailbox *, CDMBIdentity);
#endif
