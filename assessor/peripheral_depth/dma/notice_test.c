#include "dma.h"
#include "h745_cache.h"
#include <stdio.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("PD_NOTICE_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                           \
        }                                                       \
    } while (0)

int main(void)
{
    struct fw_peer peer = {9, 0, 0, {0, 0}, 0, {0, 0}};
    struct pd_notice s = {9, 0, 0, 0, 0, 0, 0, 0},
                     m = {9, 1, 1, PD_OK, 32, 32, 0x30040100, 32}, out = {0};
    CHECK(fw_peer_step(&peer, 0, 9, FW_BEGIN) == H7_OK);
    CHECK(pd_notice(&s, PD_NOTICE_PUBLISH, 9, 1, &m, 0) == PD_OK);
    CHECK(pd_notice(&s, PD_NOTICE_TAKE, 8, 1, 0, &out) == PD_STALE &&
          s.state == 1);
    m.sequence = 2;
    CHECK(pd_notice(&s, PD_NOTICE_PUBLISH, 9, 2, &m, 0) == PD_BUSY);
    CHECK(pd_notice(&s, PD_NOTICE_TAKE, 9, 1, 0, &out) == PD_OK &&
          out.complete == 32);
    CHECK(pd_notice(&s, PD_NOTICE_QUERY, 9, 1, 0, 0) == PD_BUSY &&
          fw_peer_step(&peer, 0, 9, FW_HEALTHY) == H7_BUSY);
    CHECK(pd_notice(&s, PD_NOTICE_ACK, 9, 1, 0, 0) == PD_OK &&
          pd_notice(&s, PD_NOTICE_QUERY, 9, 1, 0, 0) == PD_OK);
    CHECK(fw_peer_step(&peer, 0, 9, FW_END) == H7_OK &&
          fw_peer_step(&peer, 0, 9, FW_HEALTHY) == H7_OK);
    CHECK(pd_notice(&s, PD_NOTICE_PUBLISH, 9, 2, &m, 0) == PD_OK);
    CHECK(pd_notice(&s, PD_NOTICE_ACK, 9, 1, 0, 0) == PD_STALE);
    CHECK(pd_notice(&s, PD_NOTICE_ACK, 9, 2, 0, 0) == PD_BUSY);
    CHECK(pd_notice(&s, PD_NOTICE_TAKE, 9, 2, 0, &out) == PD_OK &&
          pd_notice(&s, PD_NOTICE_ACK, 9, 2, 0, 0) == PD_OK);
    m.sequence = UINT32_MAX;
    CHECK(pd_notice(&s, PD_NOTICE_PUBLISH, 9, UINT32_MAX, &m, 0) == PD_OK);
    CHECK(pd_notice(&s, PD_NOTICE_TAKE, 9, UINT32_MAX, 0, &out) == PD_OK &&
          pd_notice(&s, PD_NOTICE_ACK, 9, UINT32_MAX, 0, 0) == PD_OK);
    m.sequence = 1;
    CHECK(pd_notice(&s, PD_NOTICE_PUBLISH, 9, 1, &m, 0) == PD_INVALID);
    puts("PD_NOTICE_PASS");
    return 0;
}
