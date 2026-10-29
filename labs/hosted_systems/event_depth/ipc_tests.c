#include "ipc.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                \
    do                                                          \
    {                                                           \
        if (!(c))                                               \
        {                                                       \
            fprintf(stderr, "IPC-CHECK:%s:%d\n", #c, __LINE__); \
            return 1;                                           \
        }                                                       \
    } while (0)

int main(void)
{
    unsigned char bytes[8] = {0, 1, 2, 3, 128, 254, 255, 0};
    struct ei_request r;
    CHECK(ei_init(&r, bytes, 1, 1) == 0 && ei_invariant(&r));
    CHECK(ei_transition(&r, EI_QUEUE) == 0 && r.owner == EI_TRANSPORT);
    CHECK(ei_transition(&r, EI_ACCEPT) == 0 && r.owner == EI_SERVER);
    CHECK(ei_transition(&r, EI_DISCONNECT) == 0 && r.owner == EI_SERVER &&
          r.outcome == EI_UNCERTAIN);
    CHECK(ei_transition(&r, EI_CANCEL_REQUEST) == 0);
    CHECK(ei_transition(&r, EI_CANCEL_SEEN) == 0 && r.owner == EI_SERVER);
    CHECK(ei_transition(&r, EI_EFFECT) == 0 && r.result == 643);
    CHECK(ei_transition(&r, EI_STOP_CONFIRMED) == 0 &&
          r.state == EI_ABANDONED && r.owner == EI_NO_OWNER &&
          r.outcome == EI_UNCERTAIN);
    CHECK(ei_invariant(&r));
    unsigned char previous[sizeof r];
    memcpy(previous, &r, sizeof r);
    CHECK(ei_transition(&r, EI_EFFECT) == -1 &&
          memcmp(&r, previous, sizeof r) == 0);
    unsigned cuts = 0;
    for (unsigned accepted = 0; accepted < 2; ++accepted)
    {
        for (unsigned effect = 0; effect <= accepted; ++effect)
        {
            for (unsigned cancel = 0; cancel < 2; ++cancel)
            {
                CHECK(ei_init(&r, bytes, 65535, 65535) == 0);
                CHECK(ei_transition(&r, EI_QUEUE) == 0);
                if (accepted)
                {
                    CHECK(ei_transition(&r, EI_ACCEPT) == 0);
                }
                if (effect)
                {
                    CHECK(ei_transition(&r, EI_EFFECT) == 0);
                }
                CHECK(ei_transition(&r, EI_DISCONNECT) == 0);
                if (accepted)
                {
                    CHECK(r.owner == EI_SERVER && r.outcome == EI_UNCERTAIN);
                    if (cancel)
                    {
                        CHECK(ei_transition(&r, EI_CANCEL_REQUEST) == 0);
                        CHECK(ei_transition(&r, EI_CANCEL_SEEN) == 0 &&
                              r.owner == EI_SERVER);
                        CHECK(ei_transition(&r, EI_STOP_CONFIRMED) == 0);
                    }
                    else
                    {
                        if (!effect)
                        {
                            CHECK(ei_transition(&r, EI_EFFECT) == 0);
                        }
                        CHECK(ei_transition(&r, EI_COMPLETE) == 0);
                    }
                }
                CHECK(r.owner == EI_NO_OWNER && ei_invariant(&r));
                ++cuts;
            }
        }
    }
    struct ei_queue q = {0};
    struct ei_request requests[6], *out = 0;
    for (unsigned i = 0; i < 6; ++i)
    {
        CHECK(ei_init(&requests[i], bytes, 3, i + 1) == 0);
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(ei_enqueue(&q, &requests[i]) == 1);
    }
    CHECK(q.count == 4 && q.bytes == 32 && ei_enqueue(&q, &requests[4]) == 0 &&
          requests[4].owner == EI_CLIENT);
    CHECK(ei_dequeue(&q, &out) == 1 && out == &requests[0] &&
          out->owner == EI_SERVER);
    CHECK(ei_enqueue(&q, &requests[4]) == 1);
    for (unsigned i = 1; i < 5; ++i)
    {
        CHECK(ei_dequeue(&q, &out) == 1 && out == &requests[i]);
    }
    CHECK(ei_dequeue(&q, &out) == 0 && out == &requests[4] && q.bytes == 0);
    CHECK(ei_transition(&requests[0], EI_EFFECT) == 0 &&
          ei_transition(&requests[0], EI_COMPLETE) == 0);
    CHECK(ei_init(&requests[0], bytes, 3, 6) == 0);
    CHECK(ei_enqueue(&q, &requests[0]) == 1);
    CHECK(ei_enqueue(&q, &requests[5]) == -1 && requests[5].owner == EI_CLIENT);
    unsigned char reply[4];
    ei_reply(bytes, reply);
    CHECK(reply[0] == 0 && reply[1] == 0 && reply[2] == 2 && reply[3] == 131);
    printf("IPC-PASS:disconnect_cuts=%u;accepted_ack_retains_owner=1;queue_"
           "capacity=4;bytes=32;wire_sum=643;\n",
           cuts);
    return 0;
}
