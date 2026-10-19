#include "wait.h"
#include <stdio.h>

int main(void) {
  char history[64];
  while (fgets(history, sizeof(history), stdin)) {
    cmx_kernel kernel;
    CWWorld world = {0};
    CWToken tokens[2] = {{0}, {0}};
    cmx_init(&kernel, CMX_FIXED);
    if (cmx_task_add(&kernel, 3, 100, 0x1000, 256) != 0 ||
        cmx_task_add(&kernel, 5, 100, 0x1100, 256) != 1)
      return 2;
    cw_init(&world, &kernel);
    if (cw_object_init(&world, 0, CW_SEMAPHORE, 0, 1) != CMX_OK)
      return 2;
    for (unsigned i = 0; history[i] && history[i] != '\n'; ++i) {
      unsigned action = (unsigned)(history[i] - '0');
      int result;
      if (action < 2)
        result = cw_begin(&world, 0, action, cmx_at(1), 0, &tokens[action]);
      else if (action == 2)
        result = cw_post(&world, 0);
      else if (action == 3 || action == 5)
        result = cw_timeout(tokens[(action - 3) / 2], 1);
      else if (action == 4 || action == 6)
        result = cw_cancel(tokens[(action - 4) / 2]);
      else if (action == 7)
        result = cw_close(&world, 0);
      else
        return 2;
      printf("%d,%u,%u,%u", result, world.objects[0].semaphore.count,
             world.objects[0].live, kernel.now);
      for (unsigned t = 0; t < 2; ++t)
        printf(",%u,%d,%u,%llu,%u", world.nodes[t].linked,
               world.nodes[t].result, world.nodes[t].sequence,
               (unsigned long long)world.nodes[t].terminals,
               world.nodes[t].cause);
      printf(",%llu,%llu;", (unsigned long long)world.objects[0].posts,
             (unsigned long long)world.objects[0].acquired);
    }
    putchar('\n');
  }
  return 0;
}
