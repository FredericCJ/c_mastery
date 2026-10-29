#include "cl.h"
#include <stdio.h>
int main(void) {
  struct cl_queue q = {0};
  struct cl_message slots[2] = {{0}};
  int op;
  while ((op = getchar()) != EOF) {
    if (op == '\n')
      continue;
    if (op == 'Z') {
      cl_queue_init(&q, slots);
      continue;
    }
    unsigned char out[4] = {99, 99, 99, 99}, input[2] = {11, 22};
    size_t n = 99;
    uint32_t p = 99, s = 99;
    enum cl_result result;
    switch (op) {
    case 'A':
      result = cl_send(&q, 1, 1, input, 2);
      break;
    case 'B':
      result = cl_send(&q, 2, 2, 0, 0);
      break;
    case 'r':
      result = cl_recv(&q, out, 1, &n, &p, &s);
      break;
    case 'R':
      result = cl_recv(&q, out, 4, &n, &p, &s);
      break;
    case 'C':
      cl_close(&q);
      result = CL_OK;
      break;
    case 'X':
      (void)cl_abort(&q);
      result = CL_OK;
      break;
    default:
      return 2;
    }
    printf("%u %u %llu %llu %llu %zu %u %u %u %u\n", (unsigned)result, q.count,
           (unsigned long long)q.accepted, (unsigned long long)q.delivered,
           (unsigned long long)q.discarded, n, p, s, out[0], out[1]);
  }
  return 0;
}
