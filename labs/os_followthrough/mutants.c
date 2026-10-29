#include "os.h"
#include <stdio.h>
int main(void) {
  struct of_schedule r;
  os_job ties[] = {{0, 1, 1}, {0, 1, 1}}, idle = {2, 1, 1};
  if (of_schedule(ties, 2, OS_SJF, 1, 1, &r) != OF_OK)
    return 2;
  if (r.schedule.timeline[1] != 0) {
    puts("OF_KILLED:tie-selection");
    return 7;
  }
  if (of_schedule(&idle, 1, OS_FCFS, 1, 1, &r) != OF_OK)
    return 2;
  if (r.schedule.waiting[0] != 1 || r.queued[0] != 0) {
    puts("OF_KILLED:idle-as-wait");
    return 8;
  }
  puts("OF_PASS:mutant-witnesses");
  return 0;
}
