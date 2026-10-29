#ifndef CMASTERY_CONTRACT_ASYNC_H
#define CMASTERY_CONTRACT_ASYNC_H
#include "evolution_v1.h"
#include "session_v1.h"
#include "foundation.h"
enum ac_async_status { ACX_OK, ACX_INVALID, ACX_BUSY, ACX_ALLOC, ACX_PENDING,
               ACX_CANCELLED, ACX_INDETERMINATE, ACX_EXHAUSTED,
               ACX_UNIMPLEMENTED };
struct ac_box;
typedef void (*ac_terminal_fn)(void *, enum ac_async_status, struct ac_box *);
struct ac_operation {
  struct f_async core;
  struct ac_box *box;
  ac_terminal_fn terminal;
  void *context;
  unsigned committed, reporting;
};
void ac_operation_init(struct ac_operation *);
enum ac_async_status ac_submit_v2(struct ac_operation *, const unsigned char *, size_t,
                          struct ac_allocate, ac_terminal_fn, void *, unsigned inline_finish,
                          uint32_t *ticket);
enum ac_async_status ac_cancel_v2(struct ac_operation *, uint32_t);
enum ac_async_status ac_commit_v2(struct ac_operation *, uint32_t);
enum ac_async_status ac_finish_v2(struct ac_operation *, uint32_t, unsigned cancel_ack);
const unsigned char *ac_box_data(const struct ac_box *, size_t *);
void ac_box_drop(struct ac_box *);
#endif
