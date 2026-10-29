#ifndef CMASTERY_CONTRACT_OWNERSHIP_H
#define CMASTERY_CONTRACT_OWNERSHIP_H
#include "bounds.h"
enum { AC_RESOURCES=4, AC_EVENTS=64 };
struct ac_event { unsigned resource,request,acquire; };
struct ac_ledger {
  unsigned held[AC_RESOURCES], request[AC_RESOURCES];
  unsigned acquisitions[AC_RESOURCES], releases[AC_RESOURCES];
  unsigned consume_on_error[AC_RESOURCES];
  struct ac_event events[AC_EVENTS];unsigned count;
};
enum ac_status ac_resource_acquire(struct ac_ledger *,unsigned resource,unsigned request,int fail_before,int lose_ack);
enum ac_status ac_resource_query(const struct ac_ledger *,unsigned resource,unsigned request,int *owned);
enum ac_status ac_resource_close(struct ac_ledger *,unsigned resource,int fail);
struct ac_close_report { enum ac_status first;unsigned retained; };
struct ac_close_report ac_close_all(struct ac_ledger *,unsigned failure_mask);
enum ac_state { AC_NEW,AC_PARTIAL,AC_ACTIVE,AC_FAILED,AC_STOPPED,AC_DESTROYED };
struct ac_session { enum ac_state state;unsigned mask,epoch;int optional; };
enum ac_status ac_session_admit(const struct ac_session *);
enum ac_status ac_session_init(struct ac_session *,struct ac_ledger *,unsigned failure_point,int optional);
enum ac_status ac_session_stop(struct ac_session *,struct ac_ledger *);
enum ac_status ac_session_destroy(struct ac_session *);
enum ac_status ac_session_restart(struct ac_session *,struct ac_ledger *,int optional);
struct ac_token { unsigned slot,generation; };
struct ac_identity { unsigned live,generation;int value; };
struct ac_table { struct ac_identity slots[2]; };
enum ac_status ac_table_alloc(struct ac_table *,int value,struct ac_token *);
enum ac_status ac_table_free(struct ac_table *,struct ac_token);
enum ac_status ac_table_get(const struct ac_table *,struct ac_token,int *);
struct ac_shared;
struct ac_shared { unsigned strong,alive,releases;struct ac_shared *children[2]; };
enum ac_status ac_shared_retain(struct ac_shared *);
enum ac_status ac_shared_link(struct ac_shared *,struct ac_shared *);
enum ac_status ac_shared_drop(struct ac_shared *);
struct ac_lease_owner { unsigned live,issued;int value; };
enum ac_status ac_lease_read(const struct ac_lease_owner *,unsigned token_issued,unsigned now,int *);
enum ac_status ac_lease_reuse(struct ac_lease_owner *,unsigned now,int value);
struct ac_epoch_owner { struct f_buffer bytes;uint32_t values;unsigned live; };
struct ac_snapshot { struct f_view bytes;uint32_t values; };
enum ac_status ac_epoch_update(struct ac_epoch_owner *,size_t index,uint8_t value);
enum ac_status ac_epoch_snapshot(const struct ac_epoch_owner *,struct ac_snapshot *);
enum ac_status ac_epoch_read(const struct ac_epoch_owner *,struct ac_snapshot,size_t index,uint8_t *);
enum ac_status ac_epoch_reset(struct ac_epoch_owner *);
#endif
