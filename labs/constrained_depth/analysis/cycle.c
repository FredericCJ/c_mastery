#include "cmx.h"
#include <stdio.h>
#define CHECK(x)                                            \
    do                                                      \
    {                                                       \
        if (!(x))                                           \
        {                                                   \
            printf("CA_CYCLE_CHECK:%u:%s\n", __LINE__, #x); \
            return 1;                                       \
        }                                                   \
    } while (0)

int main(void)
{
    cmx_kernel k;
    cmx_mutex a, b;
    cmx_init(&k, CMX_FIXED);
    CHECK(cmx_task_add(&k, 2, 100, 0x1000, 256) == 0);
    CHECK(cmx_task_add(&k, 8, 100, 0x2000, 256) == 1);
    CHECK(cmx_mutex_init(&k, &a, 0, 0) == CMX_OK &&
          cmx_mutex_init(&k, &b, 0, 0) == CMX_OK);
    CHECK(cmx_mutex_lock(&k, &a, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_mutex_lock(&k, &b, 1, cmx_forever()) == CMX_OK);
    CHECK(cmx_mutex_lock(&k, &b, 0, cmx_forever()) == CMX_BLOCKED &&
          k.task[1].priority == 2);
    CHECK(cmx_mutex_lock(&k, &a, 1, cmx_forever()) == CMX_DEADLOCK &&
          k.task[1].state == CMX_READY);
    CHECK(cmx_mutex_unlock(&k, &b, 1) == CMX_OK && b.owner == 0 &&
          k.task[0].result == CMX_OK);
    CHECK(cmx_mutex_unlock(&k, &b, 0) == CMX_OK &&
          cmx_mutex_unlock(&k, &a, 0) == CMX_OK);
    puts(
        "CA_CYCLE_PASS:0-waits-1 proposed-1-waits-0 rejected=CMX_DEADLOCK PI-priority=2 cleanup=complete");
    return 0;
}
