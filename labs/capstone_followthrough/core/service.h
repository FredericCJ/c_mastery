#ifndef CAPSTONE_SERVICE_H
#define CAPSTONE_SERVICE_H
#include <stddef.h>
#include <stdint.h>
#include "checkpoint_depth.h"

enum
{
    CS_SESSIONS = 2,
    CS_QUEUE = 3,
    CS_WIRE = 42,
    CS_MAX_ADMISSIONS = 65535
};

enum cs_status
{
    CS_OK,
    CS_MORE,
    CS_FRAME,
    CS_REJECT,
    CS_FULL,
    CS_DENIED,
    CS_CLOSED,
    CS_PENDING,
    CS_DONE,
    CS_STALE,
    CS_INVALID,
    CS_FAILED
};

enum cs_state
{
    CS_OPEN,
    CS_DRAIN,
    CS_CANCEL,
    CS_STOPPED
};

enum cs_io_kind
{
    CS_IO_PROGRESS,
    CS_IO_AGAIN,
    CS_IO_INTERRUPTED,
    CS_IO_ERROR
};

struct cs_io
{
    size_t accepted;
    enum cs_io_kind kind;
    unsigned cause;
};
typedef struct cs_io (*cs_sink_fn)(void *, const unsigned char *, size_t);
typedef void (*cs_done_fn)(void *, uint16_t, enum cs_status);

struct cs_wire
{
    unsigned char bytes[CS_WIRE];
    size_t length;
    uint16_t sequence;
    int replaceable;
};

struct cs_input
{
    unsigned char bytes[CS_WIRE];
    size_t used;
};

struct cs_service
{
    struct cs_input input[CS_SESSIONS];
    struct cs_wire queue[CS_QUEUE], active;
    size_t head, count, capacity, offset;
    unsigned generation, limit, accepted, delivered, discarded, releases,
        rejected, denied;
    unsigned first_cause;
    int has_active, borrowed, failed;
    enum cs_state state;
};
enum cs_status cs_init(struct cs_service *, size_t, unsigned);
enum cs_status cs_offer(struct cs_service *, const struct cp_message *,
                        int authorized, int replaceable, int overwrite);
enum cs_status cs_feed(struct cs_service *, unsigned, const unsigned char *,
                       size_t, size_t *, int authorized, int replaceable);
enum cs_status cs_begin(struct cs_service *);
enum cs_status cs_step(struct cs_service *, cs_sink_fn, void *);
enum cs_status cs_recover(struct cs_service *);
enum cs_status cs_lease(struct cs_service *, const unsigned char **, size_t *,
                        unsigned *);
enum cs_status cs_complete(struct cs_service *, unsigned, int quiescent, size_t,
                           cs_done_fn, void *);
enum cs_status cs_close(struct cs_service *, int cancel);
int cs_conserved(const struct cs_service *);
/* No POSIX/register/allocator type occurs in this portable interface. */
#endif
