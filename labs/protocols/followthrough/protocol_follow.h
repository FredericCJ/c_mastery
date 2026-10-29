#ifndef PROTOCOL_FOLLOW_H
#define PROTOCOL_FOLLOW_H
#include "protocols.h"
#include "firmware_io.h"
typedef void (*pf_free_fn)(void *, void *);
struct pf_driver;

struct pf_recovery
{
    struct pf_driver *owner;
    uint32_t epoch, ticket;
};

struct pf_driver
{
    struct pt_driver protocol;
    struct fw_dma dma;
    struct fw_watchdog watchdog;
    struct fw_io io;
    unsigned char *owned;
    size_t length;
    void *free_context;
    pf_free_fn release;
    uint32_t epoch, loan_epoch;
    unsigned reset_pending, old_epoch_idle;
};

int pf_driver_init(struct pf_driver *, struct fw_io, uint32_t, pf_free_fn,
                   void *);
int pf_driver_ready(struct pf_driver *);
int pf_driver_start(struct pf_driver *, unsigned char **, size_t, unsigned,
                    struct pf_recovery *);
int pf_driver_reset(struct pf_recovery *);
int pf_driver_recover(struct pf_recovery *, uint32_t observed_epoch,
                      unsigned idle);
int pf_driver_complete(struct pf_recovery *, uint32_t observed_epoch,
                       unsigned idle);

enum
{
    PF_POLL = 0,
    PF_DMA = 1
};

struct pf_builder
{
    struct pt_builder base;
    unsigned mode_set, mode, callback;
};

struct pf_config
{
    struct pt_config base;
    unsigned mode, callback;
};

int pf_builder_mode(struct pf_builder *, unsigned, unsigned);
int pf_builder_replace(struct pf_builder *, unsigned, unsigned);
int pf_builder_finish(const struct pf_builder *, struct pf_config *);

enum
{
    PF_SEND = 0,
    PF_APPLY = 1,
    PF_ACK = 2,
    PF_RETRY = 3,
    PF_CANCEL = 4,
    PF_CRASH = 5,
    PF_RECONNECT = 6
};

struct pf_session
{
    unsigned phase, closed, crashed;
    uint32_t next, pending, last;
    int32_t pending_value, total, last_value;
};

int pf_session(struct pf_session *, unsigned, uint32_t, int32_t);
typedef void *(*pf_alloc_fn)(void *, size_t);

struct pf_decoder
{
    unsigned pc, header_pc, body_pc;
    size_t length, used;
    unsigned char checksum, *buffer;
    uint64_t deadline, last_time;
    pf_alloc_fn allocate;
    pf_free_fn release;
    void *context;
};

int pf_decoder_init(struct pf_decoder *, uint64_t, pf_alloc_fn, pf_free_fn,
                    void *);
int pf_decoder_step(struct pf_decoder *, unsigned, unsigned char, uint64_t,
                    unsigned char **, size_t *);
int pf_decoder_cancel(struct pf_decoder *);
#endif
