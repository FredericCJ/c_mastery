#ifndef CMASTERY_ARCH_H
#define CMASTERY_ARCH_H
#include <stddef.h>
#include <stdint.h>
#ifndef AR_CAP
#define AR_CAP 4
#endif
#if AR_CAP < 2 || AR_CAP > 4
#error AR_CAP must be 2 through 4
#endif
#ifndef AR_FEATURE
#define AR_FEATURE 0
#endif
enum {
  AR_OK,
  AR_INVALID,
  AR_RANGE,
  AR_FULL,
  AR_EMPTY,
  AR_PENDING,
  AR_FAILED,
  AR_CONTRACT,
  AR_CANCELLED,
  AR_STATE,
  AR_UNIMPLEMENTED
};
struct ar_record {
  int32_t value;
};
struct ar_storage {
  void *context;
  int (*get)(void *, size_t, struct ar_record *);
};
struct ar_reader {
  unsigned char bytes[5];
  size_t used;
  unsigned version;
};
struct ar_window {
  int32_t previous;
  unsigned present;
};
int ar_double(const int32_t *, size_t, int32_t *);
int ar_window(struct ar_window *, int32_t, int32_t *);
int ar_encode(unsigned, struct ar_record, unsigned char *, size_t);
int ar_decode(unsigned, const unsigned char *, size_t, struct ar_record *);
int ar_feed(struct ar_reader *, const unsigned char *, size_t, struct ar_record *);
int ar_process(struct ar_storage, size_t, int32_t *);
struct ar_item {
  unsigned id, priority;
};
struct ar_queue {
  struct ar_item items[AR_CAP];
  size_t length, capacity;
};
struct ar_eviction {
  unsigned valid;
  struct ar_item item;
};
int ar_queue_init(struct ar_queue *, size_t);
int ar_push(struct ar_queue *, struct ar_item, unsigned, struct ar_eviction *);
int ar_pop(struct ar_queue *, struct ar_item *);
struct ar_result {
  ptrdiff_t count;
  unsigned state;
};
struct ar_ops {
  void *context;
  void *(*allocate)(void *, size_t);
  void (*release)(void *, void *);
  uint64_t (*clock_ms)(void *);
  struct ar_result (*write)(void *, const unsigned char *, size_t);
  int (*notify)(void *, unsigned);
};
struct ar_task {
  struct ar_ops ops;
  unsigned char *data;
  size_t length, offset;
  uint64_t last_ms;
  unsigned active, cancelling, terminal, version, allow_partial_failure;
};
int ar_begin(struct ar_task *, struct ar_ops, const unsigned char *, size_t, unsigned,
             unsigned);
int ar_poll(struct ar_task *);
int ar_cancel(struct ar_task *);
int ar_cancel_ack(struct ar_task *);
struct ar_event {
  uint32_t incarnation, request;
  unsigned type;
};
struct ar_machine {
  uint32_t incarnation, next_request, request;
  unsigned stage, cancelling, releases;
  struct ar_event queue[2], acknowledgement;
  unsigned queued, ack_present;
};
void ar_machine_init(struct ar_machine *);
int ar_acquire(struct ar_machine *, uint32_t *);
int ar_submit(struct ar_machine *);
int ar_machine_cancel(struct ar_machine *);
int ar_event(struct ar_machine *, struct ar_event);
int ar_step(struct ar_machine *);
int ar_restart(struct ar_machine *);
struct ar_field {
  unsigned key;
  uint32_t value;
};
struct ar_config {
  uint32_t timeout_ms;
  unsigned limit, policy;
#if AR_FEATURE
  uint64_t extended_counter;
#endif
};
int ar_config(const struct ar_field *, size_t, struct ar_config *);
int ar_identity(struct ar_config, unsigned char[8]);
struct ar_supervisor {
  unsigned mode, streak, lost_logs, attempts;
};
struct ar_recovery {
  void *context;
  int (*init_a)(void *);
  int (*init_b)(void *);
  void (*drop_a)(void *);
};
int ar_supervise(struct ar_supervisor *, unsigned, int (*)(void *, unsigned), void *);
int ar_recover(struct ar_supervisor *, struct ar_recovery);
struct ar_indicator {
  void *context;
  int (*set)(void *, unsigned);
};
int ar_service(int32_t, int32_t, struct ar_indicator);
int ar_centimilli(int32_t, int32_t *);
#endif
