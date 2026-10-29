#define _GNU_SOURCE
#include "wait.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#ifndef CW_NATIVE_MUTANT
#define CW_NATIVE_MUTANT 0
#endif
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CW_CHECK:%d:%s\n", __LINE__, #x);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)

struct cl_shared {
  pthread_mutex_t lock;
  pthread_cond_t changed;
  cmx_kernel kernel;
  struct cw_world world;
  unsigned need, value, checksum, stop, failures;
  int cpus[2], observed[2];
};
struct cl_context {
  struct cl_shared *shared;
  unsigned role;
};

static int pin(int cpu) {
  cpu_set_t set, observed;
  CPU_ZERO(&set);
  CPU_SET(cpu, &set);
  if (pthread_setaffinity_np(pthread_self(), sizeof(set), &set))
    return 0;
  if (pthread_getaffinity_np(pthread_self(), sizeof(observed), &observed))
    return 0;
  return CPU_COUNT(&observed) == 1 && CPU_ISSET(cpu, &observed) &&
         sched_getcpu() == cpu;
}

static void failed(struct cl_shared *shared) {
  ++shared->failures;
  shared->stop = 1;
  pthread_cond_broadcast(&shared->changed);
}

static void *exchange(void *argument) {
  struct cl_context *context = argument;
  struct cl_shared *shared = context->shared;
  int pinned = pin(shared->cpus[context->role]);
  pthread_mutex_lock(&shared->lock);
  if (!pinned) {
    failed(shared);
    pthread_mutex_unlock(&shared->lock);
    return NULL;
  }
  shared->observed[context->role] = sched_getcpu();
  for (unsigned round = 1; round <= 1000 && !shared->stop; ++round) {
    if (context->role == 0) {
      struct cw_token token = {0};
      if (cw_begin(&shared->world, 0, 0, cmx_forever(), 0, &token) !=
          CMX_BLOCKED) {
        failed(shared);
        break;
      }
      shared->need = round;
      pthread_cond_broadcast(&shared->changed);
      while (shared->world.nodes[0].linked && !shared->stop)
        pthread_cond_wait(&shared->changed, &shared->lock);
      if (!shared->stop &&
          (shared->world.nodes[0].result != CMX_OK || shared->value != round ||
           shared->checksum != ~round)) {
        failed(shared);
        break;
      }
    } else {
      while (shared->need < round && !shared->stop)
        pthread_cond_wait(&shared->changed, &shared->lock);
      if (shared->stop)
        break;
      shared->value = round;
      if (CW_NATIVE_MUTANT != 1)
        shared->checksum = ~round;
      if (cw_post(&shared->world, 0) != CMX_OK) {
        failed(shared);
        break;
      }
      pthread_cond_broadcast(&shared->changed);
    }
  }
  pthread_mutex_unlock(&shared->lock);
  return NULL;
}

struct cl_contender {
  struct cl_shared *shared;
  pthread_barrier_t *start;
  struct cw_token token;
  unsigned event;
  int pinned;
  uint32_t now;
  int result;
};

static void *contend(void *argument) {
  struct cl_contender *context = argument;
  context->pinned = pin(context->shared->cpus[context->event % 2]);
  pthread_barrier_wait(context->start);
  pthread_mutex_lock(&context->shared->lock);
  if (context->event == 0)
    context->result = cw_ready_callback(context->token);
  else if (context->event == 1)
    context->result = cw_timeout(context->token, context->now);
  else
    context->result = cw_cancel(context->token);
  pthread_mutex_unlock(&context->shared->lock);
  return NULL;
}

int main(void) {
  struct cl_shared shared = {0};
  cpu_set_t allowed;
  CHECK(pthread_getaffinity_np(pthread_self(), sizeof(allowed), &allowed) == 0);
  unsigned found = 0;
  for (int cpu = 0; cpu < CPU_SETSIZE && found < 2; ++cpu)
    if (CPU_ISSET(cpu, &allowed))
      shared.cpus[found++] = cpu;
  CHECK(found == 2);
  CHECK(pthread_mutex_init(&shared.lock, NULL) == 0);
  CHECK(pthread_cond_init(&shared.changed, NULL) == 0);
  cmx_init(&shared.kernel, CMX_FIXED);
  CHECK(cmx_task_add(&shared.kernel, 3, 100, 0x1000, 256) == 0);
  cw_init(&shared.world, &shared.kernel);
  CHECK(cw_object_init(&shared.world, 0, CW_SEMAPHORE, 0, 1) == CMX_OK);
  struct cl_context contexts[2] = {{&shared, 0}, {&shared, 1}};
  pthread_t threads[3];
  CHECK(pthread_create(&threads[0], NULL, exchange, &contexts[0]) == 0);
  CHECK(pthread_create(&threads[1], NULL, exchange, &contexts[1]) == 0);
  CHECK(pthread_join(threads[0], NULL) == 0);
  CHECK(pthread_join(threads[1], NULL) == 0);
  CHECK(!shared.failures && shared.world.nodes[0].terminals == 1000);
  CHECK(shared.world.objects[0].posts == 1000 &&
        shared.world.objects[0].acquired == 1000);
  CHECK(shared.observed[0] == shared.cpus[0] &&
        shared.observed[1] == shared.cpus[1]);
  unsigned wins[5] = {0};
  for (unsigned round = 0; round < 100; ++round) {
    cmx_init(&shared.kernel, CMX_FIXED);
    CHECK(cmx_task_add(&shared.kernel, 3, 100, 0x1000, 256) == 0);
    cw_init(&shared.world, &shared.kernel);
    CHECK(cw_object_init(&shared.world, 0, CW_SEMAPHORE, 0, 1) == CMX_OK);
    struct cw_token token = {0};
    CHECK(cw_begin(&shared.world, 0, 0, cmx_at(1), 0, &token) == CMX_BLOCKED);
    pthread_barrier_t start;
    CHECK(pthread_barrier_init(&start, NULL, 3) == 0);
    struct cl_contender contenders[3];
    for (unsigned event = 0; event < 3; ++event) {
      contenders[event] = (struct cl_contender){&shared, &start, token, event, 0, 1, 91};
      CHECK(pthread_create(&threads[event], NULL, contend,
                           &contenders[event]) == 0);
    }
    for (unsigned event = 0; event < 3; ++event) {
      CHECK(pthread_join(threads[event], NULL) == 0);
      CHECK(contenders[event].pinned);
    }
    CHECK(pthread_barrier_destroy(&start) == 0);
    CHECK(!shared.world.nodes[0].linked &&
          shared.world.nodes[0].terminals == 1);
    CHECK(shared.world.nodes[0].cause >= CW_READY &&
          shared.world.nodes[0].cause <= CW_CANCEL);
    ++wins[shared.world.nodes[0].cause];
    CHECK(shared.world.objects[0].posts == shared.world.objects[0].acquired);
    struct cw_token next = {0};
    CHECK(cw_begin(&shared.world, 0, 0, cmx_at(2), 0, &next) == CMX_BLOCKED);
    CHECK(pthread_barrier_init(&start, NULL, 1) == 0);
    for (unsigned event = 1; event <= 2; ++event) {
      struct cl_contender stale = {&shared, &start, token, event, 0, 2, 91};
      CHECK(pthread_create(&threads[0], NULL, contend, &stale) == 0);
      CHECK(pthread_join(threads[0], NULL) == 0);
      CHECK(stale.pinned && stale.result == CW_STALE &&
            shared.world.nodes[0].linked);
    }
    CHECK(pthread_barrier_destroy(&start) == 0);
    CHECK(cw_ready_callback(next) == CMX_OK &&
          shared.world.nodes[0].terminals == 2);
  }
  CHECK(pthread_cond_destroy(&shared.changed) == 0);
  CHECK(pthread_mutex_destroy(&shared.lock) == 0);
  printf("CW_NATIVE:cpus=%d,%d;1000 publications;100 arbitration rounds;200 "
         "stale callbacks;"
         "ready=%u timeout=%u cancel=%u\n",
         shared.observed[0], shared.observed[1], wins[CW_READY],
         wins[CW_TIMEOUT], wins[CW_CANCEL]);
  return 0;
}
