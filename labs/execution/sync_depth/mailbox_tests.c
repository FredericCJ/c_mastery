#include "mailbox.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                 \
    do                                                           \
    {                                                            \
        if (!(c))                                                \
        {                                                        \
            fprintf(stderr, "CDMB_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                            \
        }                                                        \
    } while (0)

static int setup(cmx_kernel *k, struct cd_mailbox *m, unsigned capacity)
{
    cmx_init(k, CMX_FIXED);
    for (unsigned t = 0; t < 3; t++)
    {
        if (cmx_task_add(k, 9 - t, 0, 0x1000 + 0x200 * t, 256) != (int)t)
        {
            return 0;
        }
    }
    return cdmb_init(m, k, capacity, 1) == CMX_OK;
}

static int transport(void)
{
    for (unsigned capacity = 0; capacity < 2; capacity++)
    {
        cmx_kernel k;
        struct cd_mailbox m;
        struct cdmb_identity id;
        struct cdmb_request request;
        unsigned char input[4] = {11, 0, 0, 0}, reply[4] = {77, 0, 0, 0},
                      out[4] = {99, 99, 99, 99};
        CHECK(setup(&k, &m, capacity));
        CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &id) ==
              (capacity ? CMX_OK : CMX_BLOCKED));
        CHECK(cdmb_delivery(&m, 0) == (capacity ? CMX_OK : CMX_BLOCKED));
        if (capacity)
        {
            input[0] = 22; /* Mailbox's owned copy already exists. */
        }
        request = (struct cdmb_request){{77, 88, 99}, {66, 66, 66, 66}};
        CHECK(cdmb_receive(&m, &request, 3, cmx_forever()) == CMX_FULL &&
              request.id.session == 77 && request.id.generation == 88 &&
              request.id.slot == 99 && request.bytes[0] == 66);
        CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK &&
              request.bytes[0] == 11);
        CHECK(cdmb_delivery(&m, 0) == CMX_OK);
        input[0] = 44;
        CHECK(request.bytes[0] == 11);
        CHECK(cdmb_poll(&m, 0, out, 4) == CMX_BLOCKED && out[0] == 99);
        CHECK(cdmb_reply(&m, id, reply) == CMX_OK &&
              cdmb_poll(&m, 0, out, 4) == CMX_OK && out[0] == 77);
        CHECK(cdmb_poll(&m, 0, out, 4) == CMX_INVALID);
        /* Receiver-first: a raw identity match has not copied the borrowed bytes.
         */
        CHECK(setup(&k, &m, capacity));
        input[0] = 55;
        CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_BLOCKED);
        CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &id) == CMX_OK);
        CHECK(cmx_cancel(&k, 2) ==
              CMX_INVALID); /* Match already completed the kernel receive. */
        CHECK(cdmb_delivery(&m, 0) == (capacity ? CMX_OK : CMX_BLOCKED));
        CHECK(cdmb_poll(&m, 0, out, 4) == CMX_BLOCKED);
        CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK &&
              request.bytes[0] == 55);
        CHECK(cdmb_delivery(&m, 0) == CMX_OK);
        CHECK(cdmb_reply(&m, id, reply) == CMX_OK &&
              cdmb_poll(&m, 0, out, 4) == CMX_OK);
        for (unsigned after = 0; after < 2; after++)
        {
            CHECK(setup(&k, &m, capacity));
            input[0] = 11;
            CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &id) ==
                  (capacity ? CMX_OK : CMX_BLOCKED));
            if (after)
            {
                CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK &&
                      request.bytes[0] == 11);
            }
            int expected = after || capacity ? CDMB_UNKNOWN : CMX_CLOSED;
            CHECK(cdmb_cancel(&m, 0) == expected &&
                  cdmb_poll(&m, 0, out, 4) == expected);
            CHECK(cdmb_poll(&m, 0, out, 4) == CMX_INVALID);
            if (!after)
            {
                CHECK(cdmb_receive(&m, &request, 4, cmx_at(k.now)) ==
                      (capacity ? CDMB_STALE : CMX_TIMEOUT));
            }
            printf(
                "TRANSFER:capacity=%u;after=%u;terminal=%d;receiver_bytes=%u\n",
                capacity, after, expected, after ? request.bytes[0] : 0);
        }
    }
    cmx_kernel k;
    struct cd_mailbox m;
    struct cdmb_identity a, b;
    struct cdmb_request request;
    unsigned char input[4] = {11, 0, 0, 0}, out[4];
    CHECK(setup(&k, &m, 0));
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_cancel(&k, 2) == CMX_OK &&
          cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_CLOSED);
    CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &a) == CMX_BLOCKED);
    CHECK(cdmb_begin(&m, 1, input, cmx_forever(), &b) == CMX_BLOCKED);
    CHECK(k.task[0].wait == CMX_W_SEND && k.task[1].wait == CMX_W_SEND &&
          m.queue.count == 0);
    for (unsigned s = 0; s < 2; s++)
    {
        CHECK(cdmb_cancel(&m, s) == CMX_CLOSED &&
              cdmb_poll(&m, s, out, 4) == CMX_CLOSED);
    }
    puts("CDMB_TRANSPORT:capacity0-1;borrow-copy-boundaries;sender-receiver-"
         "cancel;same-direction-stall");
    return 0;
}

static int generations(void)
{
    cmx_kernel k;
    struct cd_mailbox m;
    struct cdmb_identity first, second, out_id;
    struct cdmb_request request;
    unsigned char input[4] = {1, 2, 3, 4}, reply[4] = {21, 22, 23, 24},
                  out[4] = {99, 99, 99, 99};
    CHECK(setup(&k, &m, 0));
    CHECK(cdmb_begin(&m, 0, input, cmx_at(5), &first) == CMX_BLOCKED);
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_poll(&m, 0, out, 4) == CMX_BLOCKED);
    cmx_tick(&k, 5);
    CHECK(cdmb_reply(&m, first, reply) == CDMB_STALE && !m.client[0].has_reply);
    CHECK(cdmb_poll(&m, 0, out, 4) == CDMB_UNKNOWN && out[0] == 99);
    CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &second) == CMX_BLOCKED &&
          second.generation == 2);
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_reply(&m, first, reply) == CDMB_STALE && !m.client[0].has_reply);
    CHECK(cdmb_reply(&m, second, reply) == CMX_OK);
    CHECK(cdmb_poll(&m, 0, out, 3) == CMX_FULL && out[0] == 99 &&
          m.client[0].phase == CDMB_ACTIVE);
    CHECK(cdmb_poll(&m, 0, out, 4) == CMX_OK && !memcmp(out, reply, 4));
    m.client[0].generation = CDMB_GENERATION_MAX - 1;
    CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &out_id) == CMX_BLOCKED &&
          out_id.generation == CDMB_GENERATION_MAX);
    CHECK(cdmb_cancel(&m, 0) == CMX_CLOSED &&
          cdmb_poll(&m, 0, out, 4) == CMX_CLOSED);
    out_id = (struct cdmb_identity){77, 88, 99};
    CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &out_id) == CMX_FULL &&
          out_id.session == 77 && out_id.generation == 88 && out_id.slot == 99);
    /* Reversing the selected transition changes the result: a reply wakes the
       wait before the tick, so the completed success is not timed out later. */
    CHECK(setup(&k, &m, 0));
    CHECK(cdmb_begin(&m, 0, input, cmx_at(5), &first) == CMX_BLOCKED);
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_poll(&m, 0, out, 4) == CMX_BLOCKED);
    CHECK(cdmb_reply(&m, first, reply) == CMX_OK);
    cmx_tick(&k, 5);
    CHECK(cdmb_poll(&m, 0, out, 4) == CMX_OK && !memcmp(out, reply, 4));
    /* A raw receiver-first match does not retain the borrow after a selected
       progress timeout; the delayed receive must refuse without reading it. */
    CHECK(setup(&k, &m, 0));
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_BLOCKED);
    CHECK(cdmb_begin(&m, 0, input, cmx_at(5), &first) == CMX_OK);
    CHECK(cdmb_poll(&m, 0, out, 4) == CMX_BLOCKED);
    cmx_tick(&k, 5);
    request = (struct cdmb_request){{77, 88, 99}, {66, 66, 66, 66}};
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CDMB_STALE &&
          request.id.session == 77 && request.id.generation == 88 &&
          request.id.slot == 99 && request.bytes[0] == 66 &&
          !m.client[0].borrowed);
    memset(out, 99, sizeof out);
    CHECK(cdmb_poll(&m, 0, out, 4) == CMX_TIMEOUT && out[0] == 99);
    puts(
        "CDMB_GENERATIONS:g1-timeout-g2;stale-reply-refused;short-reply-"
        "retained;65535-no-reuse;selected-timeout-before-poll;reply-before-tick;"
        "expired-raw-match-no-copy");
    return 0;
}

static int clients(void)
{
    for (unsigned capacity = 0; capacity < 2; capacity++)
    {
        cmx_kernel k;
        struct cd_mailbox m;
        struct cdmb_identity ids[2];
        struct cdmb_request request;
        unsigned char input[2][4] = {{11, 0, 0, 0}, {22, 0, 0, 0}}, out[4];
        CHECK(setup(&k, &m, capacity));
        CHECK(cdmb_begin(&m, 0, input[0], cmx_forever(), &ids[0]) ==
              (capacity ? CMX_OK : CMX_BLOCKED));
        CHECK(cdmb_begin(&m, 1, input[1], cmx_forever(), &ids[1]) ==
              CMX_BLOCKED);
        for (unsigned slot = 0; slot < 2; slot++)
        {
            CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK &&
                  request.id.slot == slot &&
                  request.bytes[0] == input[slot][0]);
            CHECK(cdmb_reply(&m, request.id, request.bytes) == CMX_OK);
            CHECK(cdmb_poll(&m, slot, out, 4) == CMX_OK &&
                  !memcmp(out, input[slot], 4));
        }
        CHECK(m.queue.count == 0 && cmx_invariant(&k));
    }
    {
        cmx_kernel k;
        struct cd_mailbox m;
        struct cdmb_identity first, failed;
        struct cdmb_request request;
        unsigned char a[4] = {11, 0, 0, 0}, b[4] = {22, 0, 0, 0},
                      out[4] = {99, 99, 99, 99};
        CHECK(setup(&k, &m, 1));
        CHECK(cdmb_begin(&m, 0, a, cmx_forever(), &first) == CMX_OK);
        uint32_t queued = m.queue.data[m.queue.head];
        CHECK(cdmb_begin(&m, 1, b, cmx_at(k.now), &failed) == CMX_TIMEOUT);
        CHECK(m.queue.count == 1 && m.queue.data[m.queue.head] == queued);
        CHECK(cdmb_poll(&m, 1, out, 4) == CMX_TIMEOUT && out[0] == 99);
        CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK &&
              request.id.slot == 0 && request.bytes[0] == 11);
        CHECK(cdmb_reply(&m, first, request.bytes) == CMX_OK &&
              cdmb_poll(&m, 0, out, 4) == CMX_OK && out[0] == 11);
    }
    puts("CDMB_CLIENTS:two-fixed-outstanding;same-capacity0-1-workload;replies-"
         "isolated");
    return 0;
}

static int unknown(void)
{
    cmx_kernel k;
    struct cd_mailbox m;
    struct cdmb_identity first, second;
    struct cdmb_request request;
    unsigned char input[4] = {1, 0, 0, 0}, reply[4] = {1, 0, 0, 0}, out[4];
    uint32_t value = 99;
    CHECK(setup(&k, &m, 0));
    CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &first) == CMX_BLOCKED);
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_effect(&m, first, 1, &value) == CMX_OK && value == 1);
    CHECK(cdmb_cancel(&m, 0) == CDMB_UNKNOWN &&
          cdmb_poll(&m, 0, out, 4) == CDMB_UNKNOWN);
    CHECK(cdmb_effect(&m, first, 1, &value) == CMX_OK && value == 1 &&
          m.receipts[0].value == 1);
    /* Deliberately unsafe application retry policy assigns a fresh ID. */
    CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &second) == CMX_BLOCKED);
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_effect(&m, second, 1, &value) == CMX_OK && value == 2);
    CHECK(cdmb_reply(&m, second, reply) == CMX_OK &&
          cdmb_poll(&m, 0, out, 4) == CMX_OK);
    puts("CDMB_UNKNOWN:stable-id-replay-effect1;fresh-id-retry-effect2;no-"
         "automatic-retry-license");
    return 0;
}

static int receipts(void)
{
    cmx_kernel k;
    struct cd_mailbox m;
    uint32_t value = 777;
    CHECK(setup(&k, &m, 0));
    for (uint32_t id = 1; id <= 4; id++)
    {
        CHECK(cdmb_effect(&m, (struct cdmb_identity){1, id, 0}, 1, &value) ==
                  CMX_OK &&
              value == id);
    }
    value = 777;
    CHECK(cdmb_effect(&m, (struct cdmb_identity){1, 5, 0}, 1, &value) ==
              CMX_FULL &&
          value == 777 && m.receipts[0].value == 4);
    CHECK(cdmb_acknowledge(&m, (struct cdmb_identity){1, 1, 0}) == CMX_OK);
    CHECK(cdmb_effect(&m, (struct cdmb_identity){1, 1, 0}, 1, &value) ==
              CDMB_STALE &&
          value == 777 && m.receipts[0].value == 4);
    CHECK(cdmb_effect(&m, (struct cdmb_identity){1, 5, 0}, 1, &value) ==
              CMX_OK &&
          value == 5);
    value = 777;
    CHECK(cdmb_effect(&m, (struct cdmb_identity){1, 2, 0}, 2, &value) ==
              CMX_INVALID &&
          value == 777);
    CHECK(cdmb_effect(&m, (struct cdmb_identity){1, 1, 1}, 9, &value) ==
              CMX_OK &&
          value == 9 && m.receipts[0].value == 5);
    puts("CDMB_RECEIPTS:four-per-client;full-refusal;ack-eviction-old-id-stale;"
         "delta-binding");
    return 0;
}

static int restart(void)
{
    cmx_kernel k;
    struct cd_mailbox m;
    struct cdmb_identity old, fresh;
    struct cdmb_request request;
    unsigned char input[4] = {1, 0, 0, 0}, reply[4] = {3, 0, 0, 0}, out[4];
    uint32_t value;
    CHECK(setup(&k, &m, 0));
    CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &old) == CMX_BLOCKED);
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_effect(&m, old, 1, &value) == CMX_OK && value == 1);
    CHECK(cdmb_poll(&m, 0, out, 4) == CMX_BLOCKED);
    CHECK(cdmb_restart(&m) == CMX_OK && m.session == 2);
    CHECK(cdmb_reply(&m, old, reply) == CDMB_STALE &&
          cdmb_effect(&m, old, 1, &value) == CDMB_STALE);
    CHECK(cdmb_poll(&m, 0, out, 4) == CDMB_UNKNOWN);
    CHECK(cdmb_begin(&m, 0, input, cmx_forever(), &fresh) == CMX_BLOCKED &&
          fresh.session == 2 && fresh.generation == 2);
    CHECK(cdmb_receive(&m, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_effect(&m, fresh, 1, &value) == CMX_OK && value == 2);
    CHECK(cdmb_reply(&m, fresh, reply) == CMX_OK &&
          cdmb_poll(&m, 0, out, 4) == CMX_OK);
    m.session = UINT32_MAX; /* Trusted near-exhaustion control fixture. */
    CHECK(cdmb_restart(&m) == CMX_FULL && m.session == UINT32_MAX &&
          m.receipts[0].value == 2);
    puts("CDMB_RESTART:session-fence;inflight-unknown;old-reply-effect-refused;"
         "counter-preserved;no-session-wrap");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "transport"))
    {
        return transport();
    }
    if (!strcmp(argv[1], "generations"))
    {
        return generations();
    }
    if (!strcmp(argv[1], "clients"))
    {
        return clients();
    }
    if (!strcmp(argv[1], "unknown"))
    {
        return unknown();
    }
    if (!strcmp(argv[1], "receipts"))
    {
        return receipts();
    }
    if (!strcmp(argv[1], "restart"))
    {
        return restart();
    }
    return 2;
}
