/*
 * Deterministic contract tests for the concurrent-state workload
 * (concurrent_state.h). They run before any measurement: a layout whose
 * final state or addresses break the contract is never measured.
 *
 * The expected final values are derived here from the request and the
 * reported tails, independently of cs_contract_holds, which is itself
 * tested against corrupted results. No event is opened.
 *
 * Output: CS_TEST_PASS on success; CS_TEST_ASSERT:<case> on stderr and exit
 * status 1 on the first failed expectation.
 */
#define _GNU_SOURCE /* sched_getaffinity, pthread_getaffinity_np */
#include "concurrent_state.h"

#include <inttypes.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void require(bool condition, const char *what)
{
    if (!condition)
    {
        (void)fprintf(stderr, "CS_TEST_ASSERT:%s\n", what);
        exit(1);
    }
}

/* The first two CPUs of the allowed set (the same CPU twice when only one
 * is allowed), and the first CPU number that is not allowed. */
static void choose_cpus(unsigned cpus[2], unsigned *disallowed)
{
    cpu_set_t allowed;
    size_t found = 0;

    CPU_ZERO(&allowed);
    require(sched_getaffinity(0, sizeof allowed, &allowed) == 0,
            "sched_getaffinity");
    *disallowed = PFD_MAX_CPU + 1u;
    for (unsigned cpu = 0; cpu <= PFD_MAX_CPU; ++cpu)
    {
        if (CPU_ISSET(cpu, &allowed) == 0)
        {
            if (*disallowed > PFD_MAX_CPU)
            {
                *disallowed = cpu;
            }
            continue;
        }
        if (found < 2u)
        {
            cpus[found++] = cpu;
        }
    }
    require(found > 0u, "at least one allowed CPU");
    if (found == 1u)
    {
        cpus[1] = cpus[0];
    }
}

static void test_names(void)
{
    static const char *const names[CS_LAYOUT_COUNT] = {"shared", "adjacent",
                                                       "separate", "solo"};
    enum cs_layout layout = CS_LAYOUT_COUNT;

    for (unsigned i = 0; i < CS_LAYOUT_COUNT; ++i)
    {
        require(cs_layout_lookup(names[i], &layout) &&
                    (layout == (enum cs_layout)i),
                "layout lookup");
        require(strcmp(cs_layout_name(layout), names[i]) == 0, "layout name");
    }
    layout = CS_LAYOUT_SOLO;
    require(!cs_layout_lookup("", &layout) &&
                !cs_layout_lookup("Shared", &layout) &&
                !cs_layout_lookup("shared ", &layout) &&
                !cs_layout_lookup("separated", &layout) &&
                !cs_layout_lookup(NULL, &layout),
            "unknown layout names rejected");
    require(layout == CS_LAYOUT_SOLO, "rejected lookup leaves output");
}

static void test_rejections(const unsigned cpus[2], unsigned disallowed)
{
    struct cs_result result;
    struct cs_request base = {.layout = CS_LAYOUT_ADJACENT,
                              .cpu = {cpus[0], cpus[1]},
                              .iterations = 16,
                              .warmup = 0};
    struct cs_request request = base;

    request.iterations = 0;
    require(cs_run(&request, &result) == CS_INVALID, "zero iterations");
    request.iterations = CS_MAX_ITERATIONS + 1u;
    require(cs_run(&request, &result) == CS_INVALID, "too many iterations");
    request = base;
    request.warmup = CS_MAX_ITERATIONS + 1u;
    require(cs_run(&request, &result) == CS_INVALID, "too long a warm-up");
    request = base;
    request.layout = CS_LAYOUT_COUNT;
    require(cs_run(&request, &result) == CS_INVALID, "unknown layout");
    request = base;
    request.cpu[1] = disallowed;
    require(cs_run(&request, &result) == CS_PLACEMENT,
            "peer CPU outside the allowed set");
    request = base;
    request.cpu[0] = disallowed;
    require(cs_run(&request, &result) == CS_PLACEMENT,
            "measured CPU outside the allowed set");
    require(cs_run(NULL, &result) == CS_INVALID, "null request");
    require(cs_run(&base, NULL) == CS_INVALID, "null result");
}

/* Independent derivation of the contract for one completed run. */
static void check_run(const struct cs_request *request,
                      const struct cs_result *result)
{
    const struct cs_thread_result *t = result->thread;
    uint64_t per_thread = request->warmup + request->iterations;
    uintptr_t a = t[0].slot_address;
    uintptr_t b = t[1].slot_address;

    require(t[0].active && (t[0].measured == request->iterations),
            "measured thread did all its window increments");
    require(t[0].window.state == PF_PMU_EMPTY, "no group was requested");
    require(t[0].window.clock_ok, "measured window clock");
    require((t[0].window.cpu_start == (int)request->cpu[0]) &&
                (t[0].window.cpu_end == (int)request->cpu[0]),
            "measured thread ran its window on its CPU");
    require(result->create_begin_ns <= result->create_return_ns &&
                result->create_return_ns <= result->join_return_ns &&
                result->create_begin_ns <= result->peer_entry_ns,
            "lifecycle timestamps ordered");
    switch (request->layout)
    {
        case CS_LAYOUT_SHARED:
            require(a == b, "shared layout uses one slot");
            require(t[0].final_value == t[1].final_value,
                    "shared slot read once");
            require(t[0].final_value == 2u * per_thread + t[0].tail + t[1].tail,
                    "shared slot counts every increment");
            break;
        case CS_LAYOUT_ADJACENT:
        case CS_LAYOUT_SEPARATE:
            if (request->layout == CS_LAYOUT_ADJACENT)
            {
                require((a != b) && (a / CS_LINE_BYTES == b / CS_LINE_BYTES),
                        "adjacent slots share one 64-byte line");
            }
            else
            {
                require(a / CS_SEPARATION_BYTES != b / CS_SEPARATION_BYTES,
                        "separate slots lie in different 128-byte blocks");
            }
            for (unsigned i = 0; i < 2u; ++i)
            {
                require(t[i].final_value == per_thread + t[i].tail,
                        "distinct slot counts its own thread exactly");
            }
            break;
        case CS_LAYOUT_SOLO:
            require(a / CS_SEPARATION_BYTES != b / CS_SEPARATION_BYTES,
                    "solo slots lie in different 128-byte blocks");
            require(!t[1].active && (t[1].measured == 0u) &&
                        (t[1].tail == 0u) && (t[1].final_value == 0u) &&
                        (t[0].tail == 0u),
                    "solo peer never increments");
            require(t[0].final_value == per_thread, "solo slot");
            break;
        default:
            require(false, "unexpected layout");
            break;
    }
    if (request->layout != CS_LAYOUT_SOLO)
    {
        require(t[1].active && (t[1].measured == request->iterations),
                "peer did all its window increments");
        require((t[1].window.cpu_start == (int)request->cpu[1]) &&
                    (t[1].window.cpu_end == (int)request->cpu[1]),
                "peer ran its window on its CPU");
    }
    require(cs_contract_holds(request, result), "contract checker accepts");
}

/* The checker must reject results that break the contract. */
static void check_corruptions(const struct cs_request *request,
                              const struct cs_result *result)
{
    struct cs_result bad = *result;

    bad.thread[0].final_value += 1u;
    require(!cs_contract_holds(request, &bad), "checker: lost increment");
    bad = *result;
    bad.thread[0].measured -= 1u;
    require(!cs_contract_holds(request, &bad), "checker: short window");
    bad = *result;
    if (request->layout == CS_LAYOUT_SHARED)
    {
        bad.thread[1].slot_address += sizeof(uint64_t);
    }
    else
    {
        bad.thread[1].slot_address = bad.thread[0].slot_address;
    }
    require(!cs_contract_holds(request, &bad), "checker: wrong addresses");
}

static void test_layouts(const unsigned cpus[2])
{
    static const uint64_t shapes[][2] = {
        {1, 0}, {1000, 0}, {4096, 64}, {65536, 4096}};
    cpu_set_t before;
    cpu_set_t after;

    CPU_ZERO(&before);
    require(pthread_getaffinity_np(pthread_self(), sizeof before, &before) == 0,
            "caller affinity before");
    for (unsigned layout = 0; layout < CS_LAYOUT_COUNT; ++layout)
    {
        for (size_t s = 0; s < sizeof shapes / sizeof shapes[0]; ++s)
        {
            struct cs_request request = {.layout = (enum cs_layout)layout,
                                         .cpu = {cpus[0], cpus[1]},
                                         .iterations = shapes[s][0],
                                         .warmup = shapes[s][1]};
            struct cs_result result;

            require(cs_run(&request, &result) == CS_OK, "run completes");
            require(result.status == CS_OK, "result status");
            check_run(&request, &result);
            check_corruptions(&request, &result);
            CPU_ZERO(&after);
            require(pthread_getaffinity_np(pthread_self(), sizeof after,
                                           &after) == 0,
                    "caller affinity after");
            require(CPU_EQUAL(&before, &after) != 0,
                    "caller affinity restored");
        }
    }
}

int main(void)
{
    unsigned cpus[2] = {0, 0};
    unsigned disallowed = 0;

    choose_cpus(cpus, &disallowed);
    test_names();
    test_rejections(cpus, disallowed);
    test_layouts(cpus);
    (void)printf("CS_TEST_PASS cpus=%u,%u\n", cpus[0], cpus[1]);
    return 0;
}
