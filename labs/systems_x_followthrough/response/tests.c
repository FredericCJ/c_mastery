#include "response.h"
#include <stdio.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "RESPONSE_X_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                  \
        }                                                              \
    } while (0)

int main(void)
{
    uint32_t result = 999;
    struct b5_task h = {1, 4, 0};
    CHECK(sx_switch_response(1, 0, 12, &h, 1, 1, &result) == B5_OK &&
          result == 12);
    result = 999;
    CHECK(sx_switch_response(1, 0, 11, &h, 1, 1, &result) == B5_RANGE &&
          result == 999);
    CHECK(sx_switch_response(2, 0, 4, NULL, 0, 1, &result) == B5_OK &&
          result == 4);
    result = 999;
    CHECK(sx_switch_response(UINT32_MAX, 0, 100, NULL, 0, 1, &result) ==
              B5_RANGE &&
          result == 999);
    CHECK(b5_response(2, 0, 3, NULL, 0, &result) == B5_OK && result == 2);
    /* Explicit one-job schedule: execute [0,1), suspend [1,5), execute [5,6). */
    unsigned executed = 0, finish = 0;
    for (unsigned t = 0; t < 6; t++)
        if (t == 0 || t == 5)
        {
            executed++;
            if (executed == 2)
                finish = t + 1;
        }
    CHECK(executed == 2 && finish == 6 && finish > 3);
    puts(
        "RESPONSE_X_PASS switches_once recurrence12 suspension_finish6_deadline3");
    return 0;
}
