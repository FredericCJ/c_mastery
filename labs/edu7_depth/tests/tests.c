#include "edu7_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "E7_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int grant(void)
{
    struct e7_grant s = {0};
    uint32_t t = 77;
    unsigned out = 99;
    CHECK(e7_grant_begin(&s, 0, &t) == E7_OK && t == 1);
    CHECK(e7_grant_first(&s, t, 199) == E7_BUSY);
    CHECK(e7_grant_first(&s, t, 1500) == E7_OK);
    CHECK(e7_grant_tick(&s, 2300) == E7_OK && s.phase == 2);
    CHECK(e7_grant_finish(&s, t, 9500) == E7_OK);
    CHECK(e7_grant_finish(&s, t, 9500) == E7_STALE);
    CHECK(e7_grant_begin(&s, 24999, &t) == E7_BUSY);
    CHECK(e7_grant_begin(&s, 25000, &t) == E7_OK && t == 2);
    CHECK(e7_grant_tick(&s, 27200) == E7_OK);
    CHECK(e7_grant_tick(&s, 27201) == E7_START_TIMEOUT && !s.phase);
    CHECK(e7_link_admit(0, 2, 2, 0, 0, 0, &out) == E7_REJECTED && out == 99);
    CHECK(e7_link_admit(0, 2, 2, 0, 1, 0, &out) == E7_OK && out == 0);
    CHECK(e7_link_admit(0, 2, 2, 1, 0, 0, &out) == E7_OK && out == 1);
    CHECK(e7_link_admit(0, 2, 1, 1, 0, 0, &out) == E7_REJECTED);
    CHECK(e7_link_admit(0, 2, 2, 1, 0, 1, &out) == E7_REJECTED);
    s.generation = UINT32_MAX;
    CHECK(e7_grant_begin(&s, 50000, &t) == E7_FULL);
    return 0;
}

static struct e7_fragment ack_for(const struct e7_fragment *f, unsigned credit)
{
    struct e7_fragment a = *f;
    a.type = 2;
    a.source = f->destination;
    a.destination = f->source;
    a.length = 0;
    a.credit = (uint8_t)credit;
    return a;
}

static int offer(void)
{
    struct e7_sender s = {0};
    uint8_t bytes[25] = {1, 2, 3};
    struct e7_fragment f, a;
    uint32_t token = 0;
    CHECK(e7_send_begin(&s, 1, 7, 8, bytes, 25, 0) == E7_OK);
    for (unsigned now = 0; now <= 200; now += 100)
    {
        CHECK(e7_send_offer(&s, now, &f, &token) == E7_OK && f.index == 0 &&
              f.length == 24);
        CHECK(e7_send_accept(&s, token, 0, now) == E7_BUSY && !s.waiting &&
              !s.retries && s.due == 0);
    }
    CHECK(e7_send_offer(&s, 250, &f, &token) == E7_OK);
    CHECK(e7_send_accept(&s, token, 1, 250) == E7_OK && s.due == 350 &&
          s.waiting && !s.retries);
    CHECK(e7_send_offer(&s, 349, &f, &token) == E7_WAIT);
    CHECK(e7_send_offer(&s, 350, &f, &token) == E7_OK);
    CHECK(e7_send_accept(&s, token, 1, 350) == E7_OK && s.retries == 1);
    a = ack_for(&f, 0);
    CHECK(e7_send_ack(&s, &a, 350) == E7_OK && s.index == 1 &&
          s.blocked_since == 350);
    CHECK(e7_send_offer(&s, 450, &f, &token) == E7_OK && f.type == 3);
    CHECK(e7_send_accept(&s, token, 1, 450) == E7_OK);
    a = ack_for(&f, 0);
    CHECK(e7_send_ack(&s, &a, 450) == E7_OK && s.blocked_since == 350);
    CHECK(e7_send_expire(&s, 849) == E7_OK && s.active);
    CHECK(e7_send_expire(&s, 850) == E7_TIMEOUT && !s.active);
    CHECK(e7_send_begin(&s, 1, 7, 8, bytes, 25, 850) == E7_STALE);
    CHECK(e7_send_begin(&s, 2, 7, 8, bytes, 25, 850) == E7_OK);
    for (unsigned now = 850; now <= 1150; now += 100)
    {
        CHECK(e7_send_offer(&s, now, &f, &token) == E7_OK);
        CHECK(e7_send_accept(&s, token, 1, now) == E7_OK);
    }
    CHECK(e7_send_offer(&s, 1250, &f, &token) == E7_TIMEOUT && !s.active);
    return 0;
}

static int session(void)
{
    struct e7_session s = {0};
    uint8_t request[] = {1, 2, 3}, reply[] = {4, 5}, cache[80] = {0};
    uint32_t token = 0;
    size_t n = 77;
    CHECK(e7_session_open(&s, 1, 1, 1, 1) == E7_OK);
    CHECK(e7_session_prepare(&s, 17, 1, 1, 1, request, 3, &token, cache, 80,
                             &n) == E7_OK &&
          token && s.receipt[0].checkpoint == 0);
    CHECK(e7_session_commit(&s, token, 0, NULL, 0) == E7_OK &&
          !s.receipt[0].checkpoint);
    CHECK(e7_session_prepare(&s, 17, 1, 1, 1, request, 3, &token, cache, 80,
                             &n) == E7_OK);
    uint32_t old = token;
    CHECK(e7_session_commit(&s, token, 1, reply, 2) == E7_OK &&
          s.receipt[0].checkpoint == 1 && !s.receipt[1].checkpoint);
    CHECK(e7_session_commit(&s, old, 1, reply, 2) == E7_STALE);
    CHECK(e7_session_prepare(&s, 17, 1, 1, 1, request, 3, &token, cache, 1,
                             &n) == E7_FULL);
    CHECK(e7_session_prepare(&s, 17, 1, 1, 1, request, 3, &token, cache, 80,
                             &n) == E7_DUPLICATE &&
          token == 0 && n == 2 && cache[1] == 5);
    request[0] = 9;
    CHECK(e7_session_prepare(&s, 17, 1, 1, 1, request, 3, &token, cache, 80,
                             &n) == E7_REJECTED);
    CHECK(e7_token_begin(&s, 17, 51) == E7_OK);
    CHECK(e7_token_complete(&s, 51, 0) == E7_BUSY && s.token_pending);
    CHECK(e7_session_prepare(&s, 17, 1, 1, 2, request, 3, &token, cache, 80,
                             &n) == E7_BUSY);
    CHECK(e7_token_complete(&s, 51, 1) == E7_OK);
    CHECK(e7_session_prepare(&s, 51, 1, 1, 1, request, 3, &token, cache, 80,
                             &n) == E7_OK);
    CHECK(e7_session_commit(&s, token, 1, reply, 2) == E7_OK);
    CHECK(e7_session_resume(&s, 1, 1, 51, 1) == E7_OK);
    CHECK(e7_session_close(&s) == E7_OK);
    CHECK(e7_session_resume(&s, 1, 1, 51, 1) == E7_STALE);
    CHECK(e7_session_open(&s, 2, 1, 1, 1) == E7_STALE);
    CHECK(e7_session_open(&s, 2, 2, 1, 1) == E7_OK &&
          !s.receipt[0].checkpoint && !s.receipt[1].checkpoint);
    s.generation = UINT32_MAX;
    CHECK(e7_session_prepare(&s, 17, 2, 2, 1, request, 3, &token, cache, 80,
                             &n) == E7_FULL);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "grant"))
        return grant();
    if (!strcmp(argv[1], "offer"))
        return offer();
    if (!strcmp(argv[1], "session"))
        return session();
    return 2;
}
