#include "wait.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CW_CHECK:%d:%s\n", __LINE__, #x);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int setup(cmx_kernel *kernel, struct cw_world *world) {
  cmx_init(kernel, CMX_FIXED);
  for (unsigned i = 0; i < 3; ++i)
    CHECK(cmx_task_add(kernel, i == 0 ? 9 : (i == 1 ? 3 : 5), 100,
                       (uintptr_t)(0x1000 + i * 0x100), 256) == (int)i);
  cw_init(world, kernel);
  return 0;
}

static int enrollment(void) {
  cmx_kernel kernel;
  struct cw_world world = {0};
  struct cw_token token = {0}, second = {0};
  CHECK(setup(&kernel, &world) == 0);
  CHECK(cw_object_init(&world, 0, CW_SEMAPHORE, 1, 2) == CMX_OK);
  CHECK(cw_begin(&world, 0, 0, cmx_at(0), 0, &token) == CMX_OK);
  CHECK(!world.nodes[0].linked && world.nodes[0].terminals == 1 &&
        !world.objects[0].semaphore.count);
  CHECK(cw_post(&world, 0) == CMX_OK);
  CHECK(cw_begin(&world, 0, 1, cmx_at(0), 1, &second) == CMX_TIMEOUT &&
        !second.owner);
  CHECK(world.objects[0].semaphore.count == 1 && !world.nodes[1].sequence);
  CHECK(cw_begin(&world, 0, 1, cmx_forever(), 0, &second) == CMX_OK);
  CHECK(cw_begin(&world, 0, 1, cmx_at(0), 0, &second) == CMX_TIMEOUT);
  CHECK(world.nodes[1].sequence == 1 && !world.nodes[1].linked);
  CHECK(cw_begin(&world, 0, 1, cmx_at(10), 0, &second) == CMX_BLOCKED);
  struct cw_token saved = second;
  CHECK(cw_begin(&world, 0, 1, cmx_forever(), 0, &second) == CMX_INVALID);
  CHECK(second.operation == saved.operation && world.nodes[1].linked);
  CHECK(cw_cancel(second) == CMX_OK);
  CHECK(cw_begin(&world, 0, 1, cmx_at(20), 0, &second) == CMX_BLOCKED);
  CHECK(cw_timeout(saved, 10) == CW_STALE && kernel.now == 0 &&
        world.nodes[1].linked);
  CHECK(cw_ready_callback(saved) == CW_STALE &&
        !world.objects[0].semaphore.count);
  CHECK(cw_cancel(saved) == CW_STALE && world.nodes[1].linked);
  CHECK(cw_cancel(second) == CMX_OK);
  CHECK(cw_exit_reap(&world, 1) == CMX_OK);
  CHECK(cmx_task_add(&kernel, 3, 100, 0x1100, 256) == 1);
  CHECK(cw_begin(&world, 0, 1, cmx_at(30), 0, &second) == CMX_BLOCKED);
  CHECK(cw_timeout(saved, 20) == CW_STALE && world.nodes[1].linked &&
        kernel.now == 0);
  CHECK(cw_close(&world, 0) == CMX_OK);
  int result = 91;
  unsigned cause = 91;
  CHECK(cw_result(second, &result, &cause) == CMX_OK && result == CMX_CLOSED &&
        cause == CW_CLOSE);
  CHECK(cw_object_init(&world, 0, CW_SEMAPHORE, 0, 1) == CMX_OK);
  CHECK(cw_result(second, &result, &cause) == CW_STALE);
  world.nodes[1].sequence = UINT32_MAX;
  second = (struct cw_token){0};
  CHECK(cw_begin(&world, 0, 1, cmx_forever(), 0, &second) == CMX_FULL &&
        !second.owner && !world.nodes[1].linked);
  world.nodes[1].sequence = 0;
  kernel.sequence = UINT32_MAX;
  CHECK(cw_begin(&world, 0, 1, cmx_forever(), 0, &second) == CMX_FULL &&
        !second.owner && !world.nodes[1].linked);
  CHECK(cmx_invariant(&kernel));
  CHECK(setup(&kernel, &world) == 0);
  CHECK(cw_object_init(&world, 0, CW_SEMAPHORE, 0, 1) == CMX_OK);
  kernel.now = UINT32_MAX - 2;
  CHECK(cw_begin(&world, 0, 0, cmx_at(2), 0, &token) == CMX_BLOCKED);
  CHECK(cw_timeout(token, 1) == CMX_INVALID && kernel.now == UINT32_MAX - 2);
  CHECK(cw_timeout(token, 2) == CMX_OK && world.nodes[0].result == CMX_TIMEOUT);
  puts("CW_ENROLL:immediate/strict deadlines;second "
       "enrollment;operation/task/object generations;sequence saturation");
  return 0;
}

static int terminal(void) {
  unsigned schedules = 0;
  for (unsigned a = 0; a < 4; ++a)
    for (unsigned b = 0; b < 4; ++b)
      for (unsigned c = 0; c < 4; ++c)
        for (unsigned d = 0; d < 4; ++d) {
          if (a == b || a == c || a == d || b == c || b == d || c == d)
            continue;
          cmx_kernel kernel;
          struct cw_world world = {0};
          struct cw_token token = {0};
          CHECK(setup(&kernel, &world) == 0);
          CHECK(cw_object_init(&world, 0, CW_SEMAPHORE, 0, 1) == CMX_OK);
          CHECK(cw_begin(&world, 0, 1, cmx_at(1), 0, &token) == CMX_BLOCKED);
          unsigned order[] = {a, b, c, d};
          int expected[] = {CMX_OK, CMX_TIMEOUT, CMX_CLOSED, CMX_CLOSED};
          unsigned causes[] = {CW_READY, CW_TIMEOUT, CW_CANCEL, CW_CLOSE};
          for (unsigned step = 0; step < 4; ++step) {
            unsigned event = order[step];
            if (event == 0)
              (void)cw_ready_callback(token);
            else if (event == 1)
              (void)cw_timeout(token, 1);
            else if (event == 2)
              (void)cw_cancel(token);
            else
              (void)cw_close(&world, 0);
            CHECK(world.nodes[1].terminals == 1 && !world.nodes[1].linked);
            CHECK(world.nodes[1].result == expected[a] &&
                  world.nodes[1].cause == causes[a]);
            CHECK(world.objects[0].posts ==
                  world.objects[0].acquired + world.objects[0].semaphore.count);
          }
          ++schedules;
        }
  /* Two consumers share one credit; a callback tagged to one enrollment
     publishes to the FIFO object, not a private entitlement to that credit. */
  cmx_kernel kernel;
  struct cw_world world = {0};
  struct cw_token first = {0}, second = {0};
  CHECK(setup(&kernel, &world) == 0);
  CHECK(cw_object_init(&world, 0, CW_SEMAPHORE, 0, 1) == CMX_OK);
  CHECK(cw_begin(&world, 0, 1, cmx_at(10), 0, &first) == CMX_BLOCKED);
  CHECK(cw_begin(&world, 0, 2, cmx_at(10), 0, &second) == CMX_BLOCKED);
  CHECK(cw_ready_callback(second) == CMX_OK && !world.nodes[1].linked &&
        world.nodes[2].linked);
  CHECK(cw_ready_callback(first) == CW_STALE);
  CHECK(cw_timeout(second, 9) == CMX_INVALID && kernel.now == 0);
  CHECK(cw_timeout(second, 10) == CMX_OK &&
        world.nodes[2].result == CMX_TIMEOUT);
  CHECK(cw_post(&world, 0) == CMX_OK && world.objects[0].semaphore.count == 1);
  CHECK(cw_post(&world, 0) == CMX_FULL);
  CHECK(world.objects[0].posts == 2 && world.objects[0].acquired == 1);
  printf("CW_TERMINAL:%u four-contender permutations;FIFO competition;token "
         "conservation;late callbacks\n",
         schedules);
  return 0;
}

static int donation(void) {
  cmx_kernel kernel;
  struct cw_world world = {0};
  struct cw_token owner = {0}, wait = {0}, lower = {0};
  CHECK(setup(&kernel, &world) == 0);
  CHECK(cw_object_init(&world, 0, CW_MUTEX, 0, 0) == CMX_OK);
  CHECK(cw_begin(&world, 0, 0, cmx_forever(), 0, &owner) == CMX_OK);
  CHECK(cw_begin(&world, 0, 1, cmx_at(10), 0, &wait) == CMX_BLOCKED &&
        kernel.task[0].priority == 3);
  CHECK(cw_begin(&world, 0, 2, cmx_forever(), 0, &lower) == CMX_BLOCKED);
  struct cw_token old = wait;
  CHECK(cw_cancel(wait) == CMX_OK && kernel.task[0].priority == 5);
  CHECK(cw_begin(&world, 0, 1, cmx_at(20), 0, &wait) == CMX_BLOCKED &&
        kernel.task[0].priority == 3);
  CHECK(cw_cancel(old) == CW_STALE && kernel.task[0].priority == 3 &&
        world.nodes[1].linked);
  CHECK(cw_timeout(old, 10) == CW_STALE && kernel.now == 0 &&
        kernel.task[0].priority == 3);
  CHECK(cw_unlock(&world, 0, 0) == CMX_OK && world.objects[0].mutex.owner == 2);
  CHECK(cw_cancel(lower) == CW_STALE && world.objects[0].mutex.owner == 2);
  CHECK(cw_timeout(wait, 20) == CMX_OK && kernel.task[2].priority == 5);
  CHECK(cw_unlock(&world, 0, 2) == CMX_OK &&
        world.objects[0].mutex.owner == CMX_NONE);
  CHECK(cw_close(&world, 0) == CMX_OK);
  CHECK(cmx_invariant(&kernel));
  CHECK(setup(&kernel, &world) == 0);
  CHECK(cw_object_init(&world, 0, CW_MUTEX, 0, 0) == CMX_OK);
  world.objects[0].acquired = UINT64_MAX - 1;
  CHECK(cw_begin(&world, 0, 0, cmx_forever(), 0, &owner) == CMX_OK &&
        world.objects[0].acquired == UINT64_MAX);
  CHECK(cw_unlock(&world, 0, CMX_TASKS) == CMX_INVALID);
  CHECK(cw_unlock(&world, 0, 1) == CMX_NOT_OWNER);
  CHECK(cw_unlock(&world, 0, 0) == CMX_OK &&
        world.objects[0].mutex.owner == CMX_NONE);
  CHECK(cw_close(&world, 0) == CMX_OK);
  CHECK(setup(&kernel, &world) == 0);
  CHECK(cw_object_init(&world, 0, CW_MUTEX, 0, 0) == CMX_OK);
  world.objects[0].acquired = UINT64_MAX - 2;
  CHECK(cw_begin(&world, 0, 0, cmx_forever(), 0, &owner) == CMX_OK);
  CHECK(cw_begin(&world, 0, 1, cmx_forever(), 0, &wait) == CMX_BLOCKED);
  CHECK(cw_begin(&world, 0, 2, cmx_forever(), 0, &lower) == CMX_BLOCKED);
  CHECK(cw_unlock(&world, 0, 0) == CMX_OK &&
        world.objects[0].acquired == UINT64_MAX && world.objects[0].mutex.owner == 1);
  CHECK(cw_unlock(&world, 0, 1) == CMX_FULL && world.objects[0].mutex.owner == 1 &&
        world.nodes[2].linked && kernel.task[1].priority == 3);
  CHECK(cw_cancel(lower) == CMX_OK && !world.nodes[2].linked);
  CHECK(cw_unlock(&world, 0, 1) == CMX_OK && cw_close(&world, 0) == CMX_OK);
  CHECK(cmx_invariant(&kernel));
  puts("CW_DONATION:actual CMX cancellation/restoration;stale callback "
       "preserves new donation;handoff ownership is not losing-operation "
       "rollback");
  return 0;
}

int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "enrollment"))
    return enrollment();
  if (!strcmp(argv[1], "terminal"))
    return terminal();
  if (!strcmp(argv[1], "donation"))
    return donation();
  return 2;
}
