#ifndef CMASTERY_MESSAGING_H
#define CMASTERY_MESSAGING_H
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
enum msg_status {
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
};

typedef int (*msg_task_fn)(void *, const atomic_bool *);
struct msg_nursery;
struct msg_child {
  pthread_t thread;
  msg_task_fn task;
  void *ctx;
  struct msg_nursery *owner;
  int result;
  bool joined;
};
struct msg_nursery {
  atomic_bool cancel;
  struct msg_child child[8];
  size_t count, attempts, fail_at;
  bool closing, closed;
};
void msg_nursery_init(struct msg_nursery *, size_t);
enum msg_status msg_spawn(struct msg_nursery *, msg_task_fn, void *);
void msg_cancel(struct msg_nursery *);
enum msg_status msg_join(struct msg_nursery *, int *);

struct msg_channel {
  unsigned values[4];
  size_t head, count, capacity;
  bool closed, offered;
  unsigned offer;
  uint64_t ticket, accepted;
};
struct msg_hub {
  pthread_mutex_t mutex;
  pthread_cond_t changed;
  struct msg_channel channel[2];
  bool stop;
  unsigned next;
};
enum msg_status msg_hub_init(struct msg_hub *, size_t, size_t);
void msg_hub_destroy(struct msg_hub *);
void msg_hub_cancel(struct msg_hub *);
enum msg_status msg_send(struct msg_hub *, unsigned, unsigned, bool);
enum msg_status msg_select(struct msg_hub *, unsigned, bool, unsigned *, unsigned *);
enum msg_status msg_close(struct msg_hub *, unsigned);

typedef void (*msg_receiver_fn)(void *, enum msg_status, unsigned);
struct msg_operation {
  enum { MSG_NEW, MSG_STARTED, MSG_DONE } state;
  bool cancel;
  msg_receiver_fn receiver;
  void *ctx;
};
void msg_connect(struct msg_operation *, msg_receiver_fn, void *);
enum msg_status msg_start(struct msg_operation *);
enum msg_status msg_stop_operation(struct msg_operation *);
enum msg_status msg_complete(struct msg_operation *, enum msg_status, unsigned);

typedef bool (*msg_handler_fn)(void *, unsigned);
struct msg_actor {
  unsigned queue[4];
  size_t head, count;
  uint64_t generation;
  bool alive, stepping;
  msg_handler_fn handler;
  void *ctx;
};
void msg_actor_init(struct msg_actor *, msg_handler_fn, void *);
enum msg_status msg_actor_send(struct msg_actor *, uint64_t, unsigned);
enum msg_status msg_actor_step(struct msg_actor *);
enum msg_status msg_actor_restart(struct msg_actor *);
enum msg_strategy { MSG_ONE, MSG_ALL, MSG_REST };
struct msg_supervisor {
  uint64_t times[4], last, window;
  size_t count, limit;
  bool escalated;
};
enum msg_status msg_supervisor_init(struct msg_supervisor *, size_t, uint64_t);
enum msg_status msg_restart_plan(struct msg_supervisor *, uint64_t, enum msg_strategy, unsigned,
                           int *, size_t *);

struct msg_deque {
  atomic_size_t top, bottom;
  atomic_uint slots[8];
};
void msg_deque_init(struct msg_deque *);
enum msg_status msg_push(struct msg_deque *, unsigned);
enum msg_status msg_pop(struct msg_deque *, unsigned *);
enum msg_status msg_steal(struct msg_deque *, unsigned *);

struct msg_cell {
  atomic_uint stamp, value;
};
struct msg_store {
  atomic_uint clock;
  struct msg_cell cells[4];
};
struct msg_txn {
  struct msg_store *store;
  unsigned snapshot, reads, writes, values[4];
  bool active;
};
void msg_store_init(struct msg_store *);
void msg_begin(struct msg_store *, struct msg_txn *);
enum msg_status msg_read(struct msg_txn *, unsigned, unsigned *);
enum msg_status msg_write(struct msg_txn *, unsigned, unsigned);
enum msg_status msg_commit(struct msg_txn *);
void msg_abort(struct msg_txn *);

struct msg_version {
  unsigned values[2];
  uint64_t stamp;
  bool live;
};
struct msg_pin {
  uint64_t stamp, generation;
  bool active;
};
struct msg_versions {
  pthread_mutex_t mutex;
  struct msg_version versions[8];
  struct msg_pin pins[4];
  uint64_t clock;
};
struct msg_snapshot {
  unsigned slot;
  uint64_t generation;
};
enum msg_status msg_versions_init(struct msg_versions *);
void msg_versions_destroy(struct msg_versions *);
enum msg_status msg_snapshot(struct msg_versions *, struct msg_snapshot *);
enum msg_status msg_snapshot_read(struct msg_versions *, struct msg_snapshot, unsigned *);
enum msg_status msg_snapshot_drop(struct msg_versions *, struct msg_snapshot);
enum msg_status msg_publish(struct msg_versions *, unsigned, unsigned);

struct msg_sequence {
  atomic_uint sequence, first, second;
};
void msg_sequence_init(struct msg_sequence *);
enum msg_status msg_sequence_write(struct msg_sequence *, unsigned, unsigned);
enum msg_status msg_sequence_read(struct msg_sequence *, size_t, unsigned *);

typedef void (*msg_next_fn)(void *, unsigned);
typedef void (*msg_terminal_fn)(void *, enum msg_status);
struct msg_stream {
  uint64_t demand;
  bool cancelled, terminal, delivering, pending_error;
  msg_next_fn next;
  msg_terminal_fn end;
  void *ctx;
};
void msg_stream_init(struct msg_stream *, msg_next_fn, msg_terminal_fn, void *);
enum msg_status msg_request(struct msg_stream *, uint64_t);
void msg_stream_cancel(struct msg_stream *);
enum msg_status msg_emit(struct msg_stream *, unsigned);
enum msg_status msg_stream_end(struct msg_stream *, enum msg_status);
#endif
