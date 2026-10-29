#include "barrier.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static cmx_kernel k;
static struct cd_barrier b;

static void semihost(uint32_t op, const void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

__attribute__((noreturn)) static void stop(unsigned status)
{
    uint32_t args[2] = {0x20026u, status};
    semihost(4, status == 0   ? "CDB_M4_PASS\n"
                : status == 1 ? "CDB_M4_CHECK\n"
                              : "CDB_M4_FAULT\n");
    semihost(0x20, args);
    for (;;)
    {
    }
}

#define CHECK(c)     \
    do               \
    {                \
        if (!(c))    \
            stop(1); \
    } while (0)

static void exercise(void)
{
    struct cdb_member h[3];
    struct cdb_ticket old[3], next[3];
    unsigned out = 999;
    cmx_init(&k, CMX_FIXED);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cmx_task_add(&k, 5 + i, 0, 0x20006000 + 256 * i, 256) == (int)i);
    }
    CHECK(cdb_init(&b, &k, 7, 8) == CMX_OK);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_register(&b, i, &h[i]) == CMX_OK);
    }
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_arrive(h[i], cmx_forever(), &old[i]) ==
              (i == 2 ? CMX_OK : CMX_BLOCKED));
    }
    CHECK(cdb_stage(&b, 5) == CMX_BLOCKED);
    CHECK(cdb_result(old[2], &out) == CMX_OK && out == 0);
    CHECK(cdb_arrive(h[2], cmx_forever(), &next[2]) == CMX_BLOCKED);
    CHECK(cdb_result(old[2], &out) == CDB_STALE);
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(cdb_result(old[i], &out) == CMX_OK && out == 0);
        CHECK(cdb_arrive(h[i], cmx_forever(), &next[i]) ==
              (i == 1 ? CMX_OK : CMX_BLOCKED));
    }
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_result(next[i], &out) == CMX_OK && out == 1);
    }
    CHECK(cdb_arrive(h[0], cmx_at(5), &next[0]) == CMX_BLOCKED &&
          cdb_arrive(h[1], cmx_forever(), &next[1]) == CMX_BLOCKED);
    cmx_tick(&k, 5);
    out = 999;
    CHECK(cdb_result(next[0], &out) == CMX_TIMEOUT && out == 999 &&
          cdb_result(next[1], &out) == CMX_BROKEN && out == 999);
    CHECK(cdb_stage(&b, 5) == CMX_OK && cdb_apply(&b) == CMX_BLOCKED);
    struct cdb_member stale = h[1];
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_release(&h[i]) == CMX_OK);
    }
    CHECK(cdb_apply(&b) == CMX_OK && cdb_register(&b, 0, &h[0]) == CMX_OK &&
          cdb_register(&b, 2, &h[2]) == CMX_OK);
    CHECK(cdb_arrive(stale, cmx_forever(), &next[1]) == CDB_STALE);
    CHECK(cdb_arrive(h[0], cmx_forever(), &next[0]) == CMX_BLOCKED &&
          cdb_cancel(next[0]) == CMX_CLOSED);
    CHECK(cdb_result(next[0], &out) == CMX_CLOSED && cdb_close(&b) == CMX_OK);
    CHECK(cdb_release(&h[0]) == CMX_OK && cdb_release(&h[2]) == CMX_OK &&
          cdb_destroy(&b) == CMX_OK);
    CHECK(cdb_init(&b, &k, 1, 3) == CMX_OK &&
          cdb_register(&b, 0, &h[0]) == CMX_OK);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_arrive(h[0], cmx_forever(), &next[0]) == CMX_OK &&
              cdb_result(next[0], &out) == CMX_OK && out == i);
    }
    CHECK(cdb_arrive(h[0], cmx_forever(), &next[0]) == CDB_RETIRED);
    CHECK(cdb_close(&b) == CMX_OK && cdb_release(&h[0]) == CMX_OK &&
          cdb_destroy(&b) == CMX_OK);
    semihost(
        4,
        "CDB_M4_LEDGER:two-rounds;timeout-broken;withdrawal;cancel;generation-retirement\n");
}

void cl_reset(void)
{
    uintptr_t source = (uintptr_t)&cl_data_load;
    for (uintptr_t p = (uintptr_t)&cl_data_start; p < (uintptr_t)&cl_data_end;
         p += 4, source += 4)
    {
        *(uint32_t *)p = *(const uint32_t *)source;
    }
    for (uintptr_t p = (uintptr_t)&cl_bss_start; p < (uintptr_t)&cl_bss_end;
         p += 4)
    {
        *(uint32_t *)p = 0;
    }
    exercise();
    stop(0);
}

static void fault(void)
{
    stop(2);
}

__attribute__((section(".vectors"), used)) const void *cdb_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault, fault, fault, fault};
