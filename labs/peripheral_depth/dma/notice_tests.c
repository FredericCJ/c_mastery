#include "dma.h"
#include <stdio.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            printf("PD_PUBLIC_NOTICE_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                                  \
        }                                                              \
    } while (0)

int main(void)
{
    struct pd_notice s = {7, 0, 0, 0, 0, 0, 0, 0},
                     m = {7, 1, 1, PD_OK, 32, 32, 0x30040100, 32}, out = {0};
    CHECK(pd_notice(&s, PD_NOTICE_PUBLISH, 7, 1, &m, 0) == PD_OK);
    CHECK(pd_notice(&s, PD_NOTICE_TAKE, 7, 1, 0, &out) == PD_OK &&
          out.complete == 32);
    CHECK(pd_notice(&s, PD_NOTICE_QUERY, 7, 1, 0, 0) == PD_BUSY);
    CHECK(pd_notice(&s, PD_NOTICE_ACK, 7, 1, 0, 0) == PD_OK);
    CHECK(pd_notice(&s, PD_NOTICE_QUERY, 7, 1, 0, 0) == PD_OK);
    puts("PD_PUBLIC_NOTICE_PASS");
    return 0;
}
