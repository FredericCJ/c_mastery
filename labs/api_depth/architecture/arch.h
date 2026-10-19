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
typedef struct {
  int32_t value;
} ArRecord;
typedef struct {
  void *context;
  int (*get)(void *, size_t, ArRecord *);
} ArStorage;
typedef struct {
  unsigned char bytes[5];
  size_t used;
  unsigned version;
} ArReader;
typedef struct {
  int32_t previous;
  unsigned present;
} ArWindow;
int ar_double(const int32_t *, size_t, int32_t *);
int ar_window(ArWindow *, int32_t, int32_t *);
int ar_encode(unsigned, ArRecord, unsigned char *, size_t);
int ar_decode(unsigned, const unsigned char *, size_t, ArRecord *);
int ar_feed(ArReader *, const unsigned char *, size_t, ArRecord *);
int ar_process(ArStorage, size_t, int32_t *);
typedef struct {
  unsigned id, priority;
} ArItem;
typedef struct {
  ArItem items[AR_CAP];
  size_t length, capacity;
} ArQueue;
typedef struct {
  unsigned valid;
  ArItem item;
} ArEviction;
int ar_queue_init(ArQueue *, size_t);
int ar_push(ArQueue *, ArItem, unsigned, ArEviction *);
int ar_pop(ArQueue *, ArItem *);
typedef struct {
  ptrdiff_t count;
  unsigned state;
} ArResult;
typedef struct {
  void *context;
  void *(*allocate)(void *, size_t);
  void (*release)(void *, void *);
  uint64_t (*clock_ms)(void *);
  ArResult (*write)(void *, const unsigned char *, size_t);
  int (*notify)(void *, unsigned);
} ArOps;
typedef struct {
  ArOps ops;
  unsigned char *data;
  size_t length, offset;
  uint64_t last_ms;
  unsigned active, cancelling, terminal, version, allow_partial_failure;
} ArTask;
int ar_begin(ArTask *, ArOps, const unsigned char *, size_t, unsigned,
             unsigned);
int ar_poll(ArTask *);
int ar_cancel(ArTask *);
int ar_cancel_ack(ArTask *);
typedef struct {
  uint32_t incarnation, request;
  unsigned type;
} ArEvent;
typedef struct {
  uint32_t incarnation, next_request, request;
  unsigned stage, cancelling, releases;
  ArEvent queue[2], acknowledgement;
  unsigned queued, ack_present;
} ArMachine;
void ar_machine_init(ArMachine *);
int ar_acquire(ArMachine *, uint32_t *);
int ar_submit(ArMachine *);
int ar_machine_cancel(ArMachine *);
int ar_event(ArMachine *, ArEvent);
int ar_step(ArMachine *);
int ar_restart(ArMachine *);
typedef struct {
  unsigned key;
  uint32_t value;
} ArField;
typedef struct {
  uint32_t timeout_ms;
  unsigned limit, policy;
#if AR_FEATURE
  uint64_t extended_counter;
#endif
} ArConfig;
int ar_config(const ArField *, size_t, ArConfig *);
int ar_identity(ArConfig, unsigned char[8]);
typedef struct {
  unsigned mode, streak, lost_logs, attempts;
} ArSupervisor;
typedef struct {
  void *context;
  int (*init_a)(void *);
  int (*init_b)(void *);
  void (*drop_a)(void *);
} ArRecovery;
int ar_supervise(ArSupervisor *, unsigned, int (*)(void *, unsigned), void *);
int ar_recover(ArSupervisor *, ArRecovery);
typedef struct {
  void *context;
  int (*set)(void *, unsigned);
} ArIndicator;
int ar_service(int32_t, int32_t, ArIndicator);
int ar_centimilli(int32_t, int32_t *);
#endif
