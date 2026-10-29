#ifndef CMASTERY_PROTOCOLS_H
#define CMASTERY_PROTOCOLS_H
#include <stddef.h>
#include <stdint.h>

enum
{
    PT_OK = 0,
    PT_INVALID = 1,
    PT_RANGE = 2,
    PT_STATE = 3,
    PT_PENDING = 4,
    PT_IO = 5,
    PT_RECOVERY = 6,
    PT_DONE = 7,
    PT_UNIMPLEMENTED = 9
};

enum
{
    PT_CLOSED = 0,
    PT_READY = 1,
    PT_RECOVERY_REQUIRED = 2
};

struct pt_device
{
    uint32_t generation;
    unsigned state, control, cleanup;
};

struct pt_closed
{
    struct pt_device *device;
    uint32_t generation;
};

struct pt_ready
{
    struct pt_device *device;
    uint32_t generation;
};

void pt_device_init(struct pt_device *, struct pt_closed *);
int pt_configure(struct pt_closed *, unsigned fail_stage, struct pt_ready *);
int pt_recover(struct pt_device *, unsigned trusted_quiescence,
               struct pt_closed *);
int pt_close(struct pt_ready *, struct pt_closed *);

struct pt_builder
{
    unsigned mask, address, length, timeout;
};

struct pt_config
{
    unsigned address, length, timeout;
};

int pt_builder_set(struct pt_builder *, unsigned field, unsigned value);
int pt_builder_finish(const struct pt_builder *, struct pt_config *);

struct pt_session
{
    unsigned role, phase, remaining;
};

enum
{
    PT_HELLO = 1,
    PT_ACCEPT = 2,
    PT_DATA = 3,
    PT_ACK = 4,
    PT_BYE = 5
};

int pt_session_step(struct pt_session *, unsigned sending, unsigned tag);

enum
{
    PT_ROOT = 0,
    PT_OFFLINE = 1,
    PT_ONLINE = 2,
    PT_IDLE = 3,
    PT_BUSY = 4,
    PT_FAULT = 5
};

enum
{
    PT_CONNECT = 0,
    PT_START = 1,
    PT_FINISH = 2,
    PT_FAIL = 3,
    PT_RESET = 4,
    PT_PING = 5
};

struct pt_machine
{
    unsigned active, dispatching, pings;
};

int pt_dispatch(struct pt_machine *, unsigned event, unsigned char *trace,
                size_t capacity, size_t *used);

struct pt_active
{
    struct pt_machine machine;
    unsigned queue[4], head, count;
};

int pt_post(struct pt_active *, unsigned event);
int pt_active_step(struct pt_active *, unsigned char *trace, size_t capacity,
                   size_t *used);

struct pt_thread
{
    unsigned pc, count;
    uint32_t sum;
};

int pt_thread_sum(struct pt_thread *, unsigned available, uint32_t value,
                  uint32_t *sum);
/* Explicit labels make duplicate labels a translation constraint, not a
 * collision hidden behind __LINE__. No automatic local survives suspension. */
#define PT_BEGIN(p)  \
    switch ((p)->pc) \
    {                \
        case 0:
#define PT_WAIT(p, label, condition) \
    do                               \
    {                                \
        (p)->pc = (label);           \
        case (label):                \
            if (!(condition))        \
                return PT_PENDING;   \
    } while (0)
#define PT_END(p)         \
    }                     \
    (p)->pc = UINT32_MAX; \
    return PT_DONE

enum
{
    PT_BT_LEAF = 0,
    PT_BT_SEQUENCE = 1,
    PT_BT_SELECTOR = 2
};

enum
{
    PT_BT_SUCCESS = 0,
    PT_BT_FAILURE = 1,
    PT_BT_RUNNING = 2
};

struct pt_node
{
    unsigned kind, first, count;
};

struct pt_behavior
{
    unsigned cursor[8], active_mask, ticking;
};

typedef unsigned (*pt_tick_fn)(void *, unsigned leaf);
typedef void (*pt_abort_fn)(void *, unsigned leaf);
int pt_behavior_tick(struct pt_behavior *, const struct pt_node *, size_t,
                     pt_tick_fn, void *, unsigned *result);
int pt_behavior_cancel(struct pt_behavior *, pt_abort_fn, void *);

struct pt_port
{
    void *context;
    int (*start_dma)(void *, const unsigned char *, size_t);
    int (*stop_and_quiesce)(void *);
    void (*watchdog)(void *);
};

struct pt_driver
{
    struct pt_device device;
    struct pt_port port;
    const unsigned char *buffer;
    size_t length;
    unsigned dma_owned;
};

int pt_driver_start(struct pt_driver *, const unsigned char *, size_t);
int pt_driver_complete(struct pt_driver *, unsigned device_error,
                       unsigned trusted_quiescence);
int pt_driver_recover(struct pt_driver *);
#endif
