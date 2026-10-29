#ifndef CMASTERY_SYNC_DEPTH_MAILBOX_H
#define CMASTERY_SYNC_DEPTH_MAILBOX_H
#include "cl.h"
#include "cmx.h"

enum
{
    CDMB_UNKNOWN = CMX_UNIMPLEMENTED + 1,
    CDMB_STALE
};

enum
{
    CDMB_IDLE,
    CDMB_ACTIVE,
    CDMB_TERMINAL
};

enum
{
    CDMB_GENERATION_MAX = 65535
};

struct cdmb_identity
{
    uint32_t session, generation;
    unsigned slot;
};

struct cdmb_request
{
    struct cdmb_identity id;
    unsigned char bytes[4];
};

struct cdmb_client
{
    uint32_t generation;
    unsigned phase, send_pending, progress_pending, delivered, received,
        has_reply;
    const unsigned char *borrowed;
    unsigned char request[4], reply[4];
    cmx_deadline deadline;
    int result;
    cmx_sem progress;
};

struct cd_mailbox
{
    cmx_kernel *kernel;
    cmx_queue queue;
    struct cdmb_client client[2];
    struct cl_ack receipts[2];
    uint32_t session;
    unsigned capacity, receive_pending;
};

int cdmb_init(struct cd_mailbox *, cmx_kernel *, unsigned capacity,
              uint32_t session);
int cdmb_begin(struct cd_mailbox *, unsigned slot,
               const unsigned char request[4], cmx_deadline,
               struct cdmb_identity *);
int cdmb_delivery(struct cd_mailbox *, unsigned slot);
int cdmb_receive(struct cd_mailbox *, struct cdmb_request *, unsigned capacity,
                 cmx_deadline);
int cdmb_reply(struct cd_mailbox *, struct cdmb_identity,
               const unsigned char reply[4]);
int cdmb_poll(struct cd_mailbox *, unsigned slot, unsigned char *reply,
              unsigned capacity);
int cdmb_cancel(struct cd_mailbox *, unsigned slot);
int cdmb_restart(struct cd_mailbox *);
int cdmb_effect(struct cd_mailbox *, struct cdmb_identity, uint32_t delta,
                uint32_t *result);
int cdmb_acknowledge(struct cd_mailbox *, struct cdmb_identity);
#endif
