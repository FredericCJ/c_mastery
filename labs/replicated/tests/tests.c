#include "replicated.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#define CHECK(condition)                                                     \
    do                                                                       \
    {                                                                        \
        if (!(condition))                                                    \
        {                                                                    \
            fprintf(stderr, "RP_CHECK line %d: %s\n", __LINE__, #condition); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int log_test(void)
{
    struct rp_log log;
    unsigned n;
    int value = 99;
    rp_log_init(&log);
    CHECK(log.count == 0 && log.arena.used == 0);
    CHECK(rp_log_append(&log, 3, 7) == RP_OK);
    CHECK(rp_log_append(&log, 3, 9) == RP_OK);
    CHECK(rp_log_get(&log, 1, 3, &value) == RP_OK && value == 7);
    CHECK(rp_log_get(&log, 2, 3, &value) == RP_OK && value == 9);
    CHECK(rp_log_get(&log, 0, 3, &value) == RP_GAP && value == 9);
    for (n = 1; n <= 3; ++n)
    {
        unsigned old_used = log.arena.used;
        log.arena.fail_after = n;
        CHECK(rp_log_append(&log, 4, 8) == RP_FAILED);
        CHECK(log.count == 2 && log.arena.used == old_used);
    }
    log.arena.fail_after = 0;
    while (log.count < 16)
        CHECK(rp_log_append(&log, log.count, 1) == RP_OK);
    CHECK(rp_log_append(&log, 1, 2) == RP_FULL && log.count == 16 &&
          log.arena.used == 48);
    return 0;
}

struct rp_shared
{
    pthread_mutex_t mutex;
    struct rp_projection projection;
    int failed;
};

static void *project_thread(void *pointer)
{
    struct rp_shared *shared = pointer;
    unsigned i;
    for (i = 1; i <= 16; ++i)
    {
        (void)pthread_mutex_lock(&shared->mutex);
        if (rp_project(&shared->projection,
                       (struct rp_event){i, i - 1, (int)i}) != RP_OK)
            shared->failed = 1;
        (void)pthread_mutex_unlock(&shared->mutex);
    }
    return NULL;
}

static int projection_test(void)
{
    struct rp_projection p = {{{0, 0, 0}}, 0, 0};
    unsigned iteration;
    CHECK(rp_project(&p, (struct rp_event){1, 0, 7}) == RP_OK && p.total == 7);
    CHECK(rp_project(&p, (struct rp_event){1, 0, 7}) == RP_OK && p.total == 7 &&
          p.count == 1);
    CHECK(rp_project(&p, (struct rp_event){1, 0, 8}) == RP_CONFLICT &&
          p.total == 7);
    CHECK(rp_project(&p, (struct rp_event){3, 0, 2}) == RP_GAP &&
          p.count == 1 && p.total == 7);
    for (iteration = 0; iteration < 128; ++iteration)
    {
        struct rp_shared shared = {
            PTHREAD_MUTEX_INITIALIZER, {{{0, 0, 0}}, 0, 0}, 0};
        pthread_t a, b;
        CHECK(!pthread_create(&a, NULL, project_thread, &shared));
        CHECK(!pthread_create(&b, NULL, project_thread, &shared));
        CHECK(!pthread_join(a, NULL));
        CHECK(!pthread_join(b, NULL));
        CHECK(!shared.failed && shared.projection.count == 16 &&
              shared.projection.total == 136);
        CHECK(!pthread_mutex_destroy(&shared.mutex));
    }
    return 0;
}

static int clocks_test(void)
{
    uint64_t stamp = 4;
    struct rp_vector a = {{1, 0, 0}}, b = {{0, 1, 0}}, v = {{9, 9, 9}};
    struct rp_hlc h = {0, 0}, remote = {10, 7};
    CHECK(rp_lamport(2, 8, &stamp) == RP_OK && stamp == 9);
    CHECK(rp_lamport(UINT64_MAX, 0, &stamp) == RP_RANGE && stamp == 9);
    CHECK(rp_vector_order(a, b) == 2);
    CHECK(rp_vector_receive(a, b, 0, &v) == RP_OK && v.v[0] == 2 &&
          v.v[1] == 1 && v.v[2] == 0);
    CHECK(rp_vector_order(a, v) == -1 && rp_vector_order(v, a) == 1 &&
          rp_vector_order(v, v) == 0);
    CHECK(rp_hlc((struct rp_hlc){10, 2}, &remote, 9, &h) == RP_OK &&
          h.physical == 10 && h.logical == 8);
    CHECK(rp_hlc(h, NULL, 8, &h) == RP_OK && h.physical == 10 &&
          h.logical == 9);
    CHECK(rp_hlc(h, NULL, 11, &h) == RP_OK && h.physical == 11 &&
          h.logical == 0);
    remote = (struct rp_hlc){11, UINT32_MAX};
    CHECK(rp_hlc(h, &remote, 0, &h) == RP_RANGE && h.logical == 0);
    return 0;
}

static int counters_test(void)
{
    struct rp_pn a = {{0}, {0}}, b = {{0}, {0}}, before;
    struct rp_register reg = {7, 0, 1};
    CHECK(rp_pn_add(&a, 0, 0, 5) == RP_OK && rp_pn_add(&b, 1, 1, 3) == RP_OK);
    rp_pn_merge(&a, &b);
    CHECK(rp_pn_value(&a) == 2);
    before = a;
    rp_pn_merge(&a, &b);
    CHECK(!memcmp(&a, &before, sizeof a));
    CHECK(rp_pn_add(&a, 0, 0, UINT32_MAX) == RP_RANGE && rp_pn_value(&a) == 2);
    CHECK(rp_register_merge(&reg, (struct rp_register){7, 2, 9}) == RP_OK &&
          reg.value == 9);
    CHECK(rp_register_merge(&reg, (struct rp_register){7, 2, 8}) ==
              RP_CONFLICT &&
          reg.value == 9);
    CHECK(rp_register_merge(&reg, (struct rp_register){6, 2, 8}) == RP_OK &&
          reg.value == 9);
    return 0;
}

static int sets_test(void)
{
    struct rp_set a = {{0}, {0}, {0}}, b = {{0}, {0}, {0}}, da, db, dr;
    unsigned i;
    CHECK(rp_set_add(&a, 0, 2, &da) == RP_OK);
    rp_set_merge(&b, &da);
    CHECK(rp_set_remove(&a, 2, &dr) == RP_OK && !rp_set_contains(&a, 2));
    CHECK(rp_set_add(&b, 1, 2, &db) == RP_OK);
    rp_set_merge(&b, &dr);
    rp_set_merge(&a, &db);
    CHECK(rp_set_contains(&a, 2) && rp_set_contains(&b, 2));
    rp_set_merge(&a, &da);
    CHECK(rp_set_contains(&a, 2));
    CHECK(rp_set_remove(&a, 2, &dr) == RP_OK);
    rp_set_merge(&b, &dr);
    rp_set_merge(&b, &db);
    CHECK(!rp_set_contains(&b, 2));
    for (i = 1; i < 8; ++i)
        CHECK(rp_set_add(&a, 0, 0, &da) == RP_OK);
    CHECK(rp_set_add(&a, 0, 0, &da) == RP_FULL && a.next[0] == 8);
    return 0;
}

static int sequence_test(void)
{
    struct rp_sequence a = {{{0, 0, 0, 0}}, 0}, b = a;
    unsigned char out[16];
    size_t n = 99;
    unsigned transformed = 99;
    CHECK(rp_sequence_insert(&a, 1, 0, 'A') == RP_OK);
    CHECK(rp_sequence_insert(&a, 2, 0, 'B') == RP_OK);
    CHECK(rp_sequence_insert(&a, 3, 1, 'C') == RP_OK);
    CHECK(rp_sequence_insert(&b, 2, 0, 'B') == RP_OK);
    CHECK(rp_sequence_insert(&b, 1, 0, 'A') == RP_OK);
    CHECK(rp_sequence_insert(&b, 3, 1, 'C') == RP_OK);
    CHECK(rp_sequence_render(&a, out, sizeof out, &n) == RP_OK && n == 3 &&
          !memcmp(out, "BAC", 3));
    CHECK(rp_sequence_render(&b, out, sizeof out, &n) == RP_OK && n == 3 &&
          !memcmp(out, "BAC", 3));
    CHECK(rp_sequence_remove(&a, 1) == RP_OK);
    CHECK(rp_sequence_render(&a, out, sizeof out, &n) == RP_OK && n == 2 &&
          !memcmp(out, "BC", 2));
    CHECK(rp_sequence_insert(&a, 3, 1, 'X') == RP_CONFLICT);
    CHECK(rp_sequence_insert(&a, 5, 4, 'D') == RP_GAP && a.count == 3);
    CHECK(rp_sequence_render(&a, out, 1, &n) == RP_FULL && n == 2);
    CHECK(rp_ot_insert(0, 2, 0, 1, &transformed) == RP_OK && transformed == 1);
    CHECK(rp_ot_insert(0, 1, 0, 2, &transformed) == RP_OK && transformed == 0);
    return 0;
}

static int merkle_test(void)
{
    static const unsigned char empty[32] = {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4,
        0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b,
        0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
    uint32_t a[3] = {1, 2, 3}, b[3] = {1, 2, 9};
    unsigned char hash[32], leaf[32], bytes[5] = {0, 0, 0, 0, 1};
    unsigned mask = 99;
    CHECK(rp_sha256(NULL, 0, hash) == RP_OK && !memcmp(hash, empty, 32));
    CHECK(rp_sha256(bytes, 5, leaf) == RP_OK &&
          rp_merkle(a, 1, hash) == RP_OK && !memcmp(hash, leaf, 32));
    CHECK(rp_merkle_diff(a, b, 3, &mask) == RP_OK && mask == 4);
    CHECK(rp_merkle_diff(a, a, 3, &mask) == RP_OK && mask == 0);
    CHECK(rp_merkle(NULL, 0, hash) == RP_OK && !memcmp(hash, empty, 32));
    return 0;
}

static int wire_test(void)
{
    unsigned char canonical[] = {8, 150, 1, 16, 1, 24, 7},
                  reordered[] = {24, 7, 16, 1, 8, 150, 1},
                  overlong[] = {8, 128, 0, 16, 1},
                  duplicate[] = {8, 1, 8, 2, 16, 1};
    struct rp_message message = {{{0, 0}}, 0};
    unsigned char out[48] = {99};
    size_t n = 99;
    CHECK(rp_decode(reordered, sizeof reordered, &message) == RP_OK &&
          message.count == 3);
    CHECK(rp_encode(&message, out, sizeof out, &n) == RP_OK &&
          n == sizeof canonical && !memcmp(out, canonical, n));
    CHECK(rp_encode(&message, out, 1, &n) == RP_FULL && n == sizeof canonical);
    CHECK(rp_decode(overlong, sizeof overlong, &message) == RP_INVALID &&
          message.count == 3);
    CHECK(rp_decode(duplicate, sizeof duplicate, &message) == RP_INVALID);
    CHECK(rp_zigzag(-1) == 1 && rp_zigzag(INT32_MIN) == UINT32_MAX &&
          rp_unzigzag(UINT32_MAX) == INT32_MIN);
    CHECK(rp_zigzag(INT32_MAX) == UINT32_MAX - 1 && rp_unzigzag(0) == 0);
    return 0;
}

static int replay_test(void)
{
    struct rp_state initial = {0, 0, 0, 1}, state = initial, changed, fresh;
    struct rp_effect effects[8], effect;
    int inputs[8] = {1, 1, 0, -1, 1, 0, 1, -1}, corrected[8];
    struct d_hamt arena;
    unsigned root = 0, old, fail;
    int value;
    CHECK(rp_step(initial, -1, &state, &effect) == RP_OK &&
          state.position == 15 && state.score == 0 &&
          state.random == 1015568748);
    CHECK(rp_step(state, 1, &state, &effect) == RP_OK && state.position == 0 &&
          state.score == 3 && effect.tick == 2);
    CHECK(rp_replay(initial, inputs, 8, &state, effects) == RP_OK &&
          state.tick == 8 && state.score == 12);
    memcpy(corrected, inputs, sizeof inputs);
    corrected[3] = 1;
    CHECK(rp_replay(initial, inputs, 3, &changed, effects) == RP_OK);
    CHECK(rp_replay(changed, corrected + 3, 5, &changed, effects) == RP_OK);
    CHECK(rp_replay(initial, corrected, 8, &fresh, effects) == RP_OK);
    CHECK(changed.position == fresh.position && changed.score == fresh.score &&
          changed.random == fresh.random);
    d_hamt_init(&arena);
    CHECK(rp_checkpoint(&arena, 0, initial, 0, &root) == RP_OK);
    old = arena.used;
    for (fail = 1; fail <= 6; ++fail)
    {
        unsigned candidate = 99;
        CHECK(rp_checkpoint(&arena, root, state, fail, &candidate) ==
                  RP_FAILED &&
              candidate == 99 && arena.used == old);
        CHECK(d_hamt_get(&arena, root, 0, &value) == D_OK && value == 0);
    }
    CHECK(rp_checkpoint(&arena, root, state, 0, &root) == RP_OK);
    CHECK(d_hamt_get(&arena, root, 0, &value) == D_OK &&
          value == (int)state.position);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "log"))
        return log_test();
    if (!strcmp(argv[1], "projection"))
        return projection_test();
    if (!strcmp(argv[1], "clocks"))
        return clocks_test();
    if (!strcmp(argv[1], "counters"))
        return counters_test();
    if (!strcmp(argv[1], "sets"))
        return sets_test();
    if (!strcmp(argv[1], "sequence"))
        return sequence_test();
    if (!strcmp(argv[1], "merkle"))
        return merkle_test();
    if (!strcmp(argv[1], "wire"))
        return wire_test();
    if (!strcmp(argv[1], "replay"))
        return replay_test();
    return 2;
}
