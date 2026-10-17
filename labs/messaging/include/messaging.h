#ifndef CMASTERY_MESSAGING_H
#define CMASTERY_MESSAGING_H
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum {
  MSG_OK,
  MSG_EMPTY,
  MSG_FULL,
  MSG_CLOSED,
  MSG_CANCELLED,
  MSG_INVALID,
  MSG_BUSY,
  MSG_STALE,
  MSG_OVERFLOW,
  MSG_CONFLICT,
  MSG_FAILED
} MsgStatus;

typedef int (*MsgTask)(void *, const atomic_bool *);
typedef struct MsgNursery MsgNursery;
typedef struct {
  pthread_t thread;
  MsgTask task;
  void *ctx;
  MsgNursery *owner;
  int result;
  bool joined;
} MsgChild;
struct MsgNursery {
  atomic_bool cancel;
  MsgChild child[8];
  size_t count, attempts, fail_at;
  bool closing, closed;
};
void msg_nursery_init(MsgNursery *, size_t);
MsgStatus msg_spawn(MsgNursery *, MsgTask, void *);
void msg_cancel(MsgNursery *);
MsgStatus msg_join(MsgNursery *, int *);

typedef struct {
  unsigned values[4];
  size_t head, count, capacity;
  bool closed, offered;
  unsigned offer;
  uint64_t ticket, accepted;
} MsgChannel;
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t changed;
  MsgChannel channel[2];
  bool stop;
  unsigned next;
} MsgHub;
MsgStatus msg_hub_init(MsgHub *, size_t, size_t);
void msg_hub_destroy(MsgHub *);
void msg_hub_cancel(MsgHub *);
MsgStatus msg_send(MsgHub *, unsigned, unsigned, bool);
MsgStatus msg_select(MsgHub *, unsigned, bool, unsigned *, unsigned *);
MsgStatus msg_close(MsgHub *, unsigned);

typedef void (*MsgReceiver)(void *, MsgStatus, unsigned);
typedef struct {
  enum { MSG_NEW, MSG_STARTED, MSG_DONE } state;
  bool cancel;
  MsgReceiver receiver;
  void *ctx;
} MsgOperation;
void msg_connect(MsgOperation *, MsgReceiver, void *);
MsgStatus msg_start(MsgOperation *);
MsgStatus msg_stop_operation(MsgOperation *);
MsgStatus msg_complete(MsgOperation *, MsgStatus, unsigned);

typedef bool (*MsgHandler)(void *, unsigned);
typedef struct {
  unsigned queue[4];
  size_t head, count;
  uint64_t generation;
  bool alive, stepping;
  MsgHandler handler;
  void *ctx;
} MsgActor;
void msg_actor_init(MsgActor *, MsgHandler, void *);
MsgStatus msg_actor_send(MsgActor *, uint64_t, unsigned);
MsgStatus msg_actor_step(MsgActor *);
MsgStatus msg_actor_restart(MsgActor *);
typedef enum { MSG_ONE, MSG_ALL, MSG_REST } MsgStrategy;
typedef struct {
  uint64_t times[4], last, window;
  size_t count, limit;
  bool escalated;
} MsgSupervisor;
MsgStatus msg_supervisor_init(MsgSupervisor *, size_t, uint64_t);
MsgStatus msg_restart_plan(MsgSupervisor *, uint64_t, MsgStrategy, unsigned,
                           int *, size_t *);

typedef struct {
  atomic_size_t top, bottom;
  atomic_uint slots[8];
} MsgDeque;
void msg_deque_init(MsgDeque *);
MsgStatus msg_push(MsgDeque *, unsigned);
MsgStatus msg_pop(MsgDeque *, unsigned *);
MsgStatus msg_steal(MsgDeque *, unsigned *);

typedef struct {
  atomic_uint stamp, value;
} MsgCell;
typedef struct {
  atomic_uint clock;
  MsgCell cells[4];
} MsgStore;
typedef struct {
  MsgStore *store;
  unsigned snapshot, reads, writes, values[4];
  bool active;
} MsgTxn;
void msg_store_init(MsgStore *);
void msg_begin(MsgStore *, MsgTxn *);
MsgStatus msg_read(MsgTxn *, unsigned, unsigned *);
MsgStatus msg_write(MsgTxn *, unsigned, unsigned);
MsgStatus msg_commit(MsgTxn *);
void msg_abort(MsgTxn *);

typedef struct {
  unsigned values[2];
  uint64_t stamp;
  bool live;
} MsgVersion;
typedef struct {
  uint64_t stamp, generation;
  bool active;
} MsgPin;
typedef struct {
  pthread_mutex_t mutex;
  MsgVersion versions[8];
  MsgPin pins[4];
  uint64_t clock;
} MsgVersions;
typedef struct {
  unsigned slot;
  uint64_t generation;
} MsgSnapshot;
MsgStatus msg_versions_init(MsgVersions *);
void msg_versions_destroy(MsgVersions *);
MsgStatus msg_snapshot(MsgVersions *, MsgSnapshot *);
MsgStatus msg_snapshot_read(MsgVersions *, MsgSnapshot, unsigned *);
MsgStatus msg_snapshot_drop(MsgVersions *, MsgSnapshot);
MsgStatus msg_publish(MsgVersions *, unsigned, unsigned);

typedef struct {
  atomic_uint sequence, first, second;
} MsgSequence;
void msg_sequence_init(MsgSequence *);
MsgStatus msg_sequence_write(MsgSequence *, unsigned, unsigned);
MsgStatus msg_sequence_read(MsgSequence *, size_t, unsigned *);

typedef void (*MsgNext)(void *, unsigned);
typedef void (*MsgTerminal)(void *, MsgStatus);
typedef struct {
  uint64_t demand;
  bool cancelled, terminal, delivering, pending_error;
  MsgNext next;
  MsgTerminal end;
  void *ctx;
} MsgStream;
void msg_stream_init(MsgStream *, MsgNext, MsgTerminal, void *);
MsgStatus msg_request(MsgStream *, uint64_t);
void msg_stream_cancel(MsgStream *);
MsgStatus msg_emit(MsgStream *, unsigned);
MsgStatus msg_stream_end(MsgStream *, MsgStatus);
#endif
