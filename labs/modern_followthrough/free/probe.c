#include "higher_order.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "MF_CHECK:%d:%s\n", __LINE__, #x); exit(1); } } while (0)

typedef struct { unsigned calls; int mode; } Context;
static int identity(HOA *a, int32_t value, void *opaque, unsigned *out)
{
    Context *c = opaque;
    ++c->calls;
    int status = ho_pure(a, value, out);
    if (status != HO_OK) return status;
    if (c->mode == 1) return HO_FAILED;
    if (c->mode == 2) *out = 0;
    return HO_OK;
}

static int same_node(HONode a, HONode b)
{
    return a.kind == b.kind && a.value == b.value && a.left == b.left && a.right == b.right;
}

static void full(void)
{
    for (int value = -1; value <= 1; ++value) {
        HOA arena = {0};
        unsigned root;
        CHECK(ho_pure(&arena, value, &root) == HO_OK);
        for (unsigned i = 0; i < 63; ++i) {
            unsigned next;
            CHECK(ho_choose(&arena, root, root, &next) == HO_OK);
            root = next;
        }
        HOSummary summary = {91, 92, 93};
        int status = ho_summarize(&arena, root, &summary);
        printf("summary %d %d %" PRIu64 " %" PRIu64 " %" PRId64 "\n",
               value, status, summary.paths, summary.emissions, summary.sum);
        if (value == 1) CHECK(status == HO_RANGE && summary.paths == 91 && summary.emissions == 92 && summary.sum == 93);
        else CHECK(status == HO_OK && summary.paths == (UINT64_C(1) << 63) && summary.emissions == 0 && summary.sum == (value ? INT64_MIN : 0));
        HOA before = arena;
        Context context = {0, 0};
        unsigned output = 999;
        status = ho_bind(&arena, root, identity, &context, &output);
        CHECK(status == HO_FULL && output == 999 && arena.used == 64);
        CHECK(context.calls == 1 && arena.attempts == before.attempts + 1);
        for (unsigned i = 1; i <= 64; ++i) CHECK(same_node(arena.nodes[i], before.nodes[i]));
        unsigned char choices[63] = {0};
        HOResult result;
        CHECK(ho_run(&arena, root, choices, 63, &result) == HO_OK);
        CHECK(result.value == value && result.count == 0 && result.consumed == 63);
        printf("full %d %d %u %u %zu %u\n", value, status, output, arena.used,
               arena.attempts - before.attempts, context.calls);
    }
}

static void failures(void)
{
    for (unsigned cut = 0; cut <= 6; ++cut) {
        HOA arena = {0};
        unsigned pure, choice, root;
        CHECK(ho_pure(&arena, 5, &pure) == HO_OK);
        CHECK(ho_choose(&arena, pure, pure, &choice) == HO_OK);
        CHECK(ho_emit(&arena, 7, choice, &root) == HO_OK);
        HOA before = arena;
        Context context = {0, cut == 5 ? 1 : cut == 6 ? 2 : 0};
        if (cut > 0 && cut < 5) arena.fail_at = arena.attempts + cut;
        unsigned output = 777;
        int status = ho_bind(&arena, root, identity, &context, &output);
        if (cut == 0) CHECK(status == HO_OK && output == 7 && arena.used == 7);
        else CHECK(status == (cut == 6 ? HO_INVALID : HO_FAILED) && output == 777 && arena.used == 3);
        for (unsigned i = 1; i <= 3; ++i) CHECK(same_node(arena.nodes[i], before.nodes[i]));
        for (unsigned char answer = 0; answer <= 1; ++answer) {
            HOResult result;
            CHECK(ho_run(&arena, root, &answer, 1, &result) == HO_OK);
            CHECK(result.value == 5 && result.count == 1 && result.trace[0] == 7 && result.consumed == 1);
        }
        printf("cut %u %d %u %u %zu %u\n", cut, status, output, arena.used,
               arena.attempts - before.attempts, context.calls);
    }
}

int main(void)
{
    full();
    failures();
    return 0;
}
