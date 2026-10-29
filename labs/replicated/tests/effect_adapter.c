/* Original local receiver: requests are synthetic counters, never external effects. */
#include "replicated.h"
#include <stdio.h>

struct rp_receiver
{
    unsigned seen, count, score[8];
};

static int apply(struct rp_receiver *receiver, struct rp_effect effect,
                 unsigned fail_tick)
{
    unsigned bit;
    if (effect.tick < 1 || effect.tick > 8)
        return RP_INVALID;
    bit = 1u << (effect.tick - 1);
    if (receiver->seen & bit)
        return receiver->score[effect.tick - 1] == effect.score ? RP_OK
                                                                : RP_CONFLICT;
    if (effect.tick == fail_tick)
        return RP_FAILED;
    receiver->score[effect.tick - 1] = effect.score;
    receiver->seen |= bit;
    ++receiver->count;
    return RP_OK;
}

int main(void)
{
    const int moves[8] = {1, 1, 0, -1, 1, 0, 1, -1};
    struct rp_effect effects[8];
    struct rp_state state;
    struct rp_receiver receiver = {0, 0, {0}};
    unsigned i, unsafe_count = 0;
    if (rp_replay((struct rp_state){0, 0, 0, 1}, moves, 8, &state, effects) !=
        RP_OK)
        return 1;
    /* A naive adapter performs all requests again on replay. */
    for (i = 0; i < 16; ++i)
        ++unsafe_count;
    for (i = 0; i < 8; ++i)
        if (apply(&receiver, effects[i], 4) != RP_OK)
            break;
    if (receiver.count != 3)
        return 2;
    for (i = 0; i < 8; ++i)
        if (apply(&receiver, effects[i], 0) != RP_OK)
            return 3;
    for (i = 0; i < 8; ++i)
        if (apply(&receiver, effects[i], 0) != RP_OK)
            return 4;
    if (receiver.count != 8 || unsafe_count != 16)
        return 5;
    effects[0].score = 99;
    if (apply(&receiver, effects[0], 0) != RP_CONFLICT || receiver.count != 8)
        return 6;
    puts(
        "unsafe_replay_effects=16 deduplicated_effects=8 failure_prefix=3 conflicting_reuse=rejected");
    return 0;
}
