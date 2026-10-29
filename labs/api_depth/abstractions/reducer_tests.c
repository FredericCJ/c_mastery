#include "reducer.h"
#include <stdio.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "REDUCER-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

static int same(struct am_transition a, struct am_transition b)
{
    return a.next.state == b.next.state && a.next.sequence == b.next.sequence &&
           a.effect.kind == b.effect.kind && a.effect.key == b.effect.key;
}

static unsigned released[3];

struct ac_backend
{
    unsigned id, calls;
    struct am_backend backend;
};

static enum as_status execute(void *ctx, struct am_effect e)
{
    struct ac_backend *b = ctx;
    ++b->calls;
    return am_execute(&b->backend, e);
}

static void release(void *ctx)
{
    struct ac_backend *b = ctx;
    ++released[b->id];
}

int main(void)
{
    unsigned cells = 0;
    const unsigned sparse[4] = {11, 29, 71, 101};
    const int next_state[4][6] = {{1, -1, -1, -1, -1, -1},
                                  {-1, 2, -1, -1, -1, -1},
                                  {-1, -1, 1, 2, 3, 3},
                                  {-1, -1, 1, 2, -1, -1}};
    for (unsigned s = 0; s < 4; ++s)
        for (unsigned seq = 0; seq < 8; ++seq)
            for (int e = -1; e <= 6; ++e)
            {
                struct am_machine machine = {(enum am_state)s, seq};
                struct am_transition sentinel = {{AM_FAILED, 99},
                                                 {AM_WRITE, 99}},
                                     a = sentinel, b = sentinel, d = sentinel,
                                     f = sentinel;
                int valid = !(s == AM_IDLE && seq) &&
                            !((s == AM_WAIT || s == AM_FAILED) && !seq) &&
                            e >= 0 && e < 6;
                enum as_status expected =
                    !valid || next_state[s][e] < 0
                        ? AS_INVALID
                        : ((s == AM_READY && e == AM_SEND && seq == 7) ? AS_FULL
                                                                       : AS_OK);
                CHECK(am_switch(machine, (enum am_event)e, &a) == expected &&
                      am_table(machine, (enum am_event)e, &b) == expected &&
                      am_functions(machine, (enum am_event)e, &d) == expected &&
                      am_sparse(sparse[s], seq, (enum am_event)e, &f) ==
                          expected);
                CHECK(same(a, b) && same(b, d) && same(d, f));
                if (expected != AS_OK)
                    CHECK(same(a, sentinel));
                else
                {
                    unsigned key = (e == AM_SEND) ? seq + 1 : seq;
                    int effect = e == AM_SEND || e == AM_TIMEOUT;
                    CHECK(a.next.state == (enum am_state)next_state[s][e] &&
                          a.next.sequence == key);
                    CHECK(a.effect.kind == (effect ? AM_WRITE : AM_NO_EFFECT) &&
                          a.effect.key == (effect ? key : 0));
                }
                ++cells;
            }
    struct am_transition out;
    CHECK(am_sparse(70, 1, AM_ACK, &out) == AS_INVALID);
    CHECK(am_table((struct am_machine){(enum am_state)99, 0}, AM_SEND, &out) ==
          AS_INVALID);
    for (unsigned fault = 0; fault < 3; ++fault)
    {
        struct am_machine state = {AM_IDLE, 0};
        struct am_backend backend = {0};
        CHECK(am_switch(state, AM_START, &out) == AS_OK);
        state = out.next;
        CHECK(am_switch(state, AM_SEND, &out) == AS_OK);
        state = out.next;
        backend.fail_before = fault == 1;
        backend.lose_ack = fault == 2;
        enum as_status effect = am_execute(&backend, out.effect);
        if (fault)
        {
            CHECK(effect == AS_FAILED &&
                  am_switch(state, AM_EFFECT_FAILURE, &out) == AS_OK);
            state = out.next;
            CHECK(state.state == AM_FAILED &&
                  am_switch(state, AM_TIMEOUT, &out) == AS_OK &&
                  out.effect.key == 1);
            state = out.next;
            CHECK(am_execute(&backend, out.effect) == AS_OK);
        }
        else
            CHECK(effect == AS_OK);
        CHECK(am_switch(state, AM_ACK, &out) == AS_OK &&
              out.next.state == AM_READY);
        CHECK(backend.value == 1 && backend.committed_key == 1 &&
              backend.attempts == (fault ? 2u : 1u));
    }
    struct ac_backend backends[3] = {0};
    for (unsigned i = 0; i < 3; ++i)
        backends[i].id = i;
    struct am_hot hot = {{execute, release, backends, 1}, {0}, 0, 0};
    struct am_lease old, now;
    CHECK(am_hot_acquire(&hot, &old) == AS_OK);
    CHECK(am_hot_replace(
              &hot, (struct am_strategy){execute, release, backends + 1, 2}) ==
              AS_OK &&
          !released[0]);
    CHECK(am_hot_acquire(&hot, &now) == AS_OK);
    CHECK(old.selected.execute(old.selected.context,
                               (struct am_effect){AM_WRITE, 1}) == AS_OK &&
          now.selected.execute(now.selected.context,
                               (struct am_effect){AM_WRITE, 1}) == AS_OK);
    CHECK(backends[0].calls == 1 && backends[1].calls == 1);
    CHECK(am_hot_replace(
              &hot, (struct am_strategy){execute, release, backends + 2, 3}) ==
              AS_INVALID &&
          !released[0] && !released[1]);
    CHECK(am_hot_close(&hot) == AS_INVALID);
    CHECK(am_hot_release(&hot, &old) == AS_OK && released[0] == 1 &&
          am_hot_release(&hot, &old) == AS_INVALID);
    CHECK(am_hot_replace(
              &hot, (struct am_strategy){execute, release, backends + 2, 3}) ==
              AS_OK &&
          !released[1]);
    CHECK(am_hot_release(&hot, &now) == AS_OK && released[1] == 1);
    CHECK(am_hot_close(&hot) == AS_OK && released[2] == 1 &&
          am_hot_close(&hot) == AS_OK);
    CHECK(am_hot_replace(&hot, (struct am_strategy){execute, release, backends,
                                                    4}) == AS_INVALID);
    struct am_hot limited = {{execute, release, backends, 4}, {0}, 0, 0};
    struct am_lease leases[4] = {0};
    for (unsigned i = 0; i < 3; ++i)
        CHECK(am_hot_acquire(&limited, leases + i) == AS_OK);
    CHECK(am_hot_acquire(&limited, leases + 3) == AS_INVALID &&
          !leases[3].live);
    for (unsigned i = 0; i < 3; ++i)
        CHECK(am_hot_release(&limited, leases + i) == AS_OK);
    CHECK(am_hot_close(&limited) == AS_OK && released[0] == 2);
    printf(
        "REDUCER-PASS:cells=%u;effect_histories=3;hot_backends=3;machine=%zu;transition=%zu;hot=%zu;lease=%zu\n",
        cells, sizeof(struct am_machine), sizeof(struct am_transition),
        sizeof(struct am_hot), sizeof(struct am_lease));
    return 0;
}
