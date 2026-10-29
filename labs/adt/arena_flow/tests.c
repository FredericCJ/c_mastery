#define _POSIX_C_SOURCE 200809L
#include "arena_flow.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "ADO_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int certificate(void)
{
    struct ad_node nodes[4] = {
        {0, 7, {0}, {0}}, {0, 9, {0}, {0}}, {1, 0, {0}, {1}}, {1, 0, {2}, {2}}};
    struct adf_arena a;
    CHECK(adf_arena(nodes, 4, &a) == ADF_OK);
    uint32_t value = 99;
    CHECK(adf_root(&a, (struct ad_node_index){3}, &value) == ADF_OK &&
          value == 32);
    nodes[0].value = 100;
    CHECK(adf_root(&a, (struct ad_node_index){3}, &value) == ADF_OK &&
          value == 32);
    nodes[3].kind = 2;
    CHECK(adf_arena(nodes, 4, &a) == ADF_INVALID && a.values[3] == 32);
    nodes[3].kind = 1;
    nodes[3].left.index = 4;
    CHECK(adf_arena(nodes, 4, &a) == ADF_INVALID && a.values[3] == 32);
    value = 99;
    CHECK(adf_root(&a, (struct ad_node_index){4}, &value) == ADF_INVALID &&
          value == 99);
    CHECK(adf_arena(NULL, 0, &a) == ADF_INVALID && a.values[3] == 32);
    puts("ADF_ARENA whole-owned-certificate");
    return 0;
}

static int worklist(void)
{
    struct adf_node graph[3] = {{2, 0, 255}, {4, 1, 255}, {1, 0, 255}};
    struct adf_session s;
    struct adf_result out;
    CHECK(adf_start(graph, 3, &s) == ADF_OK);
    CHECK(adf_step(&s, 520, &out) == ADF_OK && out.values[0] == 1 &&
          out.values[1] == 1 && out.values[2] == 1);
    struct adf_node merge[3] = {{0, 1, 255}, {0, 2, 255}, {3, 0, 255}};
    CHECK(adf_start(merge, 3, &s) == ADF_OK);
    CHECK(adf_step(&s, 520, &out) == ADF_OK && out.values[2] == 3);
    merge[2].keep = 1;
    CHECK(adf_start(merge, 3, &s) == ADF_OK);
    CHECK(adf_step(&s, 520, &out) == ADF_OK && out.values[2] == 1);
    struct adf_node invalid[2] = {{0, 1, 255}, {4, 0, 255}};
    unsigned old = s.values[2];
    CHECK(adf_start(invalid, 2, &s) == ADF_INVALID && s.values[2] == old);
    struct adf_node reverse[8];
    for (unsigned i = 0; i < 8; ++i)
    {
        reverse[i] = (struct adf_node){1u << ((i + 7) % 8), 1u << i,
                                       255u ^ (1u << ((i + 3) % 8))};
    }
    CHECK(adf_start(reverse, 8, &s) == ADF_OK);
    for (unsigned i = 0; i < 8; ++i)
    {
        reverse[i] = (struct adf_node){0, 256, 0};
    }
    out = (struct adf_result){{333}, 444};
    enum adf_status status = ADF_BUDGET;
    unsigned calls = 0;
    while (status == ADF_BUDGET && calls < 520)
    {
        status = adf_step(&s, 1, &out);
        ++calls;
        if (status == ADF_BUDGET)
        {
            CHECK(out.values[0] == 333 && out.evaluations == 444);
        }
    }
    CHECK(status == ADF_OK);
    unsigned expected[8] = {241, 227, 199, 143, 31, 62, 124, 248};
    for (unsigned i = 0; i < 8; ++i)
    {
        CHECK(out.values[i] == expected[i]);
    }
    CHECK(out.evaluations == calls);
    puts("ADF_FLOW cyclic-union-reschedule owned-reverse-ring");
    return 0;
}

static int outcomes(void)
{
    struct adf_node cycle[2] = {{2, 0, 255}, {1, 3, 255}};
    struct adf_session s;
    struct adf_result out = {{777}, 888};
    CHECK(adf_start(cycle, 2, &s) == ADF_OK);
    CHECK(adf_step(&s, 0, &out) == ADF_BUDGET && out.values[0] == 777 &&
          out.evaluations == 888 && s.evaluations == 0);
    CHECK(adf_step(&s, 1, &out) == ADF_BUDGET && out.values[0] == 777 &&
          s.evaluations == 1);
    CHECK(adf_step(&s, 520, &out) == ADF_OK && out.values[0] == 3 &&
          out.values[1] == 3);
    unsigned count = out.evaluations;
    CHECK(adf_step(&s, 0, &out) == ADF_OK && out.evaluations == count);
    struct adf_boolean never = {0, 0, 5};
    unsigned assignment = 99;
    CHECK(adf_boolean(&never, 1, 0, &assignment) == ADF_BUDGET &&
          assignment == 99);
    CHECK(adf_boolean(&never, 1, 1, &assignment) == ADF_BUDGET &&
          assignment == 99);
    CHECK(adf_boolean(&never, 1, 2, &assignment) == ADF_NO_FIXED_POINT &&
          assignment == 99);
    struct adf_boolean two[2] = {{1, 1, 5}, {0, 0, 5}};
    CHECK(adf_boolean(two, 2, 1, &assignment) == ADF_BUDGET &&
          assignment == 99);
    CHECK(adf_boolean(two, 2, 4, &assignment) == ADF_OK &&
          assignment ==
              1); /* synchronous00->11->00 oscillates although01/10 are fixed */
    unsigned x = 0, y = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
        unsigned nx = !y, ny = !x;
        x = nx;
        y = ny;
    }
    CHECK(x == 0 && y == 0);
    puts("ADF_OUTCOME budget-not-no-fixed-point oscillation-with-solutions");
    return 0;
}

static int baseline(void)
{
    struct ad_node nodes[32] = {
        {0, 7, {0}, {0}}, {0, 9, {0}, {0}}, {1, 0, {0}, {1}}};
    uint32_t out = 99;
    CHECK(ad_eval(nodes, 3, (struct ad_node_index){2}, &out) && out == 16);
    nodes[2].left.index = 2;
    CHECK(!ad_eval(nodes, 3, (struct ad_node_index){0}, &out) && out == 16);
    nodes[2].left.index = 0;
    nodes[0].value = UINT32_MAX;
    CHECK(!ad_eval(nodes, 3, (struct ad_node_index){0}, &out) && out == 16);
    nodes[0].value = 1;
    for (unsigned i = 1; i < 32; ++i)
    {
        nodes[i] = (struct ad_node){1, 0, {i - 1}, {i - 1}};
    }
    CHECK(ad_eval(nodes, 32, (struct ad_node_index){31}, &out) &&
          out == UINT32_C(2147483648));
    CHECK(!ad_eval(nodes, 0, (struct ad_node_index){0}, &out));
    puts("ADF_BASE actual-whole-arena");
    return 0;
}

static int arena_oracle(void)
{
    unsigned n, root;
    while (scanf("%u %u", &n, &root) == 2)
    {
        if (n > 33)
        {
            return 2;
        }
        struct ad_node nodes[33];
        for (unsigned i = 0; i < n; ++i)
        {
            if (scanf("%u %" SCNu32 " %u %u", &nodes[i].kind, &nodes[i].value,
                      &nodes[i].left.index, &nodes[i].right.index) != 4)
            {
                return 2;
            }
        }
        uint32_t base = 777, cached = 777;
        int ok = ad_eval(nodes, n, (struct ad_node_index){root}, &base);
        struct adf_arena arena = {0};
        enum adf_status ctor = adf_arena(nodes, n, &arena), get = ADF_INVALID;
        if (ctor == ADF_OK)
        {
            get = adf_root(&arena, (struct ad_node_index){root}, &cached);
        }
        printf("%d %" PRIu32 " %u %u %" PRIu32 "\n", ok, base, (unsigned)ctor,
               (unsigned)get, cached);
    }
    return 0;
}

static int flow_oracle(void)
{
    unsigned n, budget;
    while (scanf("%u %u", &n, &budget) == 2)
    {
        if (!n || n > 8)
        {
            return 2;
        }
        struct adf_node nodes[8];
        for (unsigned i = 0; i < n; ++i)
        {
            if (scanf("%u %u %u", &nodes[i].predecessors, &nodes[i].generate,
                      &nodes[i].keep) != 3)
            {
                return 2;
            }
        }
        struct adf_session s;
        enum adf_status ctor = adf_start(nodes, n, &s);
        printf("%u", (unsigned)ctor);
        if (ctor == ADF_OK)
        {
            struct adf_result result;
            for (unsigned i = 0; i < 8; ++i)
            {
                result.values[i] = 333;
            }
            result.evaluations = 444;
            enum adf_status first = adf_step(&s, budget, &result);
            unsigned preserved = result.evaluations == 444;
            for (unsigned i = 0; i < 8; ++i)
            {
                preserved &= result.values[i] == 333;
            }
            printf(" %u %u %u %u", (unsigned)first, s.pending, s.evaluations,
                   preserved);
            for (unsigned i = 0; i < n; ++i)
            {
                printf(" %u", s.values[i]);
            }
            unsigned turns = 0;
            enum adf_status last = first;
            while (last == ADF_BUDGET && turns < 520)
            {
                last = adf_step(&s, 1, &result);
                ++turns;
            }
            printf(" %u %u %u", (unsigned)last, s.pending, result.evaluations);
            for (unsigned i = 0; i < n; ++i)
            {
                printf(" %u", result.values[i]);
            }
            CHECK(last == ADF_OK);
            struct adf_result repeated = result;
            CHECK(adf_step(&s, 0, &repeated) == ADF_OK &&
                  repeated.evaluations == result.evaluations);
        }
        puts("");
    }
    return 0;
}

static int boolean_oracle(void)
{
    unsigned n, budget;
    while (scanf("%u %u", &n, &budget) == 2)
    {
        if (!n || n > 4)
        {
            return 2;
        }
        struct adf_boolean nodes[4];
        for (unsigned i = 0; i < n; ++i)
        {
            if (scanf("%u %u %u", &nodes[i].left, &nodes[i].right,
                      &nodes[i].table) != 3)
            {
                return 2;
            }
        }
        unsigned result = 99;
        enum adf_status s = adf_boolean(nodes, n, budget, &result);
        printf("%u %u\n", (unsigned)s, result);
    }
    return 0;
}

static uint32_t tree(const struct ad_node *nodes, unsigned i, unsigned *visits)
{
    ++*visits;
    return nodes[i].kind ? tree(nodes, nodes[i].left.index, visits) +
                               tree(nodes, nodes[i].right.index, visits)
                         : nodes[i].value;
}

static double seconds(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        return 0;
    }
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static int metrics(void)
{
    struct ad_node nodes[16] = {{0, 1, {0}, {0}}};
    for (unsigned i = 1; i < 16; ++i)
    {
        nodes[i] = (struct ad_node){1, 0, {i - 1}, {i - 1}};
    }
    unsigned visits = 0;
    CHECK(tree(nodes, 15, &visits) == 32768 && visits == 65535);
    struct adf_arena a;
    CHECK(adf_arena(nodes, 16, &a) == ADF_OK);
    double base[7], cached[7], flow[7];
    uint64_t checksum = 0, flow_checksum = 0;
    unsigned evaluations = 0;
    struct adf_node ring[8];
    for (unsigned i = 0; i < 8; ++i)
    {
        ring[i] = (struct adf_node){1u << ((i + 1) % 8), 1u << i, 255};
    }
    for (unsigned k = 0; k < 7; ++k)
    {
        double t = seconds();
        checksum = 0;
        for (unsigned i = 0; i < 10000; ++i)
        {
            uint32_t x;
            CHECK(ad_eval(nodes, 16, (struct ad_node_index){15}, &x));
            checksum += x;
        }
        base[k] = seconds() - t;
        t = seconds();
        CHECK(adf_arena(nodes, 16, &a) == ADF_OK);
        checksum = 0;
        for (unsigned i = 0; i < 10000; ++i)
        {
            uint32_t x;
            CHECK(adf_root(&a, (struct ad_node_index){15}, &x) == ADF_OK);
            checksum += x;
        }
        cached[k] = seconds() - t;
        t = seconds();
        flow_checksum = 0;
        for (unsigned i = 0; i < 1000; ++i)
        {
            struct adf_session s;
            struct adf_result r;
            CHECK(adf_start(ring, 8, &s) == ADF_OK);
            CHECK(adf_step(&s, 520, &r) == ADF_OK);
            for (unsigned j = 0; j < 8; ++j)
            {
                flow_checksum += r.values[j];
            }
            evaluations = r.evaluations;
        }
        flow[k] = seconds() - t;
    }
    printf(
        "{\"iterations\":10000,\"nodes\":16,\"tree_visits\":%u,\"checksum\":%" PRIu64
        ",\"arena_size\":%zu,\"arena_alignment\":%zu,\"session_size\":%zu,\"flow_iterations\":1000,\"flow_checksum\":%" PRIu64
        ",\"flow_evaluations\":%u,\"base_seconds\":[",
        visits, checksum, sizeof(struct adf_arena), _Alignof(struct adf_arena),
        sizeof(struct adf_session), flow_checksum, evaluations);
    for (unsigned i = 0; i < 7; ++i)
    {
        printf("%s%.9f", i ? "," : "", base[i]);
    }
    printf("],\"cached_seconds\":[");
    for (unsigned i = 0; i < 7; ++i)
    {
        printf("%s%.9f", i ? "," : "", cached[i]);
    }
    printf("],\"flow_seconds\":[");
    for (unsigned i = 0; i < 7; ++i)
    {
        printf("%s%.9f", i ? "," : "", flow[i]);
    }
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "arena_certificate"))
    {
        return certificate();
    }
    if (!strcmp(argv[1], "arena_worklist"))
    {
        return worklist();
    }
    if (!strcmp(argv[1], "arena_outcomes"))
    {
        return outcomes();
    }
    if (!strcmp(argv[1], "arena_base"))
    {
        return baseline();
    }
    if (!strcmp(argv[1], "arena_oracle"))
    {
        return arena_oracle();
    }
    if (!strcmp(argv[1], "flow_oracle"))
    {
        return flow_oracle();
    }
    if (!strcmp(argv[1], "boolean_oracle"))
    {
        return boolean_oracle();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        return metrics();
    }
    return 2;
}
