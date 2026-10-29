#ifndef CMASTERY_ABSTRACTION_SESSION_H
#define CMASTERY_ABSTRACTION_SESSION_H
#include "ownership.h"
#include "session_v1.h"

enum as_status
{
    AS_OK,
    AS_INVALID,
    AS_FULL,
    AS_UNSUPPORTED,
    AS_FAILED,
    AS_UNIMPLEMENTED
};

enum as_state
{
    AS_NEW,
    AS_ACTIVE,
    AS_SUSPENDED,
    AS_STOPPED
};

enum as_event
{
    AS_START,
    AS_INCREMENT,
    AS_STOP,
    AS_SUSPEND,
    AS_RESUME,
    AS_CANCEL
};

struct as_session
{
    enum as_state state;
    unsigned count, ready;
    struct ac_ledger *ledger;
};

typedef void (*as_construction_observer_fn)(void *, struct as_session *);
void as_session_init(struct as_session *, struct ac_ledger *);
enum as_status as_construct(struct as_session *, unsigned fail_second,
                            as_construction_observer_fn, void *);
enum as_status as_event(struct as_session *, enum as_event);
struct as_opaque;
enum as_status as_opaque_create(struct ac_allocate, struct ac_ledger *,
                                struct as_opaque **);
enum as_status as_opaque_event(struct as_opaque *, enum as_event);
enum as_status as_opaque_read(const struct as_opaque *, unsigned *);
enum as_status as_opaque_destroy(struct as_opaque *);

struct as_header
{
    uint32_t version, flags;
    size_t bytes;
};
typedef enum as_status (*as_run_fn)(void *, enum as_event);
typedef enum as_status (*as_flush_fn)(void *);

struct as_ops_v1
{
    struct as_header header;
    as_run_fn run;
};

struct as_ops_v2
{
    struct as_ops_v1 prefix;
    as_flush_fn flush;
};

struct as_backend
{
    as_run_fn run;
    as_flush_fn flush;
    void *context;
};

struct as_extra
{
    struct as_session *session;
    unsigned calls;
};
extern const struct as_ops_v1 as_direct_ops;
extern const struct as_ops_v2 as_adapted_ops;
enum as_status as_backend(const void *descriptor, size_t available,
                          void *context, struct as_backend *);
enum as_status as_backend_run(struct as_backend, enum as_event);
enum as_status as_backend_flush(struct as_backend);
#endif
