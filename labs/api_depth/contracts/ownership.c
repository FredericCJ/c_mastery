#include "ownership.h"

/* Intentional learner scaffold, not an unfinished assessor solution. */
struct ac_close_report ac_close_all(struct ac_ledger *l, unsigned f)
{
    (void)l;
    (void)f;
    return (struct ac_close_report){AC_NOT_IMPLEMENTED, 0};
}

enum ac_status ac_resource_acquire(struct ac_ledger *l, unsigned id,
                                   unsigned request, int fail, int lost)
{
    (void)l;
    (void)id;
    (void)request;
    (void)fail;
    (void)lost;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_resource_query(const struct ac_ledger *l, unsigned id,
                                 unsigned request, int *out)
{
    (void)l;
    (void)id;
    (void)request;
    (void)out;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_resource_close(struct ac_ledger *l, unsigned id, int fail)
{
    (void)l;
    (void)id;
    (void)fail;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_session_admit(const struct ac_session *s)
{
    (void)s;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_session_init(struct ac_session *s, struct ac_ledger *l,
                               unsigned fail, int optional)
{
    (void)s;
    (void)l;
    (void)fail;
    (void)optional;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_session_stop(struct ac_session *s, struct ac_ledger *l)
{
    (void)s;
    (void)l;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_session_destroy(struct ac_session *s)
{
    (void)s;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_session_restart(struct ac_session *s, struct ac_ledger *l,
                                  int optional)
{
    (void)s;
    (void)l;
    (void)optional;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_table_alloc(struct ac_table *t, int v, struct ac_token *out)
{
    (void)t;
    (void)v;
    (void)out;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_table_get(const struct ac_table *t, struct ac_token h,
                            int *out)
{
    (void)t;
    (void)h;
    (void)out;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_table_free(struct ac_table *t, struct ac_token h)
{
    (void)t;
    (void)h;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_shared_retain(struct ac_shared *s)
{
    (void)s;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_shared_link(struct ac_shared *parent, struct ac_shared *child)
{
    (void)parent;
    (void)child;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_shared_drop(struct ac_shared *s)
{
    (void)s;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_lease_read(const struct ac_lease_owner *o, unsigned token,
                             unsigned now, int *out)
{
    (void)o;
    (void)token;
    (void)now;
    (void)out;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_lease_reuse(struct ac_lease_owner *o, unsigned now, int value)
{
    (void)o;
    (void)now;
    (void)value;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_epoch_update(struct ac_epoch_owner *o, size_t i,
                               uint8_t value)
{
    (void)o;
    (void)i;
    (void)value;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_epoch_snapshot(const struct ac_epoch_owner *o,
                                 struct ac_snapshot *out)
{
    (void)o;
    (void)out;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_epoch_read(const struct ac_epoch_owner *o,
                             struct ac_snapshot s, size_t i, uint8_t *out)
{
    (void)o;
    (void)s;
    (void)i;
    (void)out;
    return AC_NOT_IMPLEMENTED;
}

enum ac_status ac_epoch_reset(struct ac_epoch_owner *o)
{
    (void)o;
    return AC_NOT_IMPLEMENTED;
}
