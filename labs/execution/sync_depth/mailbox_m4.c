#include "mailbox.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static cmx_kernel kernel;
static struct cd_mailbox mailbox;

static void semihost(uint32_t op, const void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

__attribute__((noreturn)) static void stop(unsigned status)
{
    uint32_t arguments[2] = {0x20026u, status};
    semihost(4, status == 0   ? "CDMB_M4_PASS\n"
                : status == 1 ? "CDMB_M4_CHECK\n"
                              : "CDMB_M4_FAULT\n");
    semihost(0x20, arguments);
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
    cmx_init(&kernel, CMX_FIXED);
    for (unsigned t = 0; t < 3; t++)
    {
        CHECK(cmx_task_add(&kernel, 9 - t, 0, 0x20006000 + 0x200 * t, 256) ==
              (int)t);
    }
    CHECK(cdmb_init(&mailbox, &kernel, 0, 1) == CMX_OK);
    unsigned char input[4] = {11, 0, 0, 0}, reply[4] = {22, 0, 0, 0},
                  out[4] = {99, 99, 99, 99};
    struct cdmb_identity first, second;
    struct cdmb_request request;
    uint32_t value = 777;
    CHECK(cdmb_begin(&mailbox, 0, input, cmx_forever(), &first) == CMX_BLOCKED);
    CHECK(cdmb_delivery(&mailbox, 0) == CMX_BLOCKED);
    CHECK(cdmb_receive(&mailbox, &request, 4, cmx_forever()) == CMX_OK &&
          request.bytes[0] == 11);
    CHECK(cdmb_cancel(&mailbox, 0) == CDMB_UNKNOWN &&
          cdmb_poll(&mailbox, 0, out, 4) == CDMB_UNKNOWN);
    CHECK(cdmb_begin(&mailbox, 0, input, cmx_forever(), &second) ==
          CMX_BLOCKED);
    CHECK(cdmb_receive(&mailbox, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_reply(&mailbox, first, reply) == CDMB_STALE);
    CHECK(cdmb_reply(&mailbox, second, reply) == CMX_OK);
    CHECK(cdmb_poll(&mailbox, 0, out, 3) == CMX_FULL &&
          mailbox.client[0].phase == CDMB_ACTIVE && out[0] == 99);
    CHECK(cdmb_poll(&mailbox, 0, out, 4) == CMX_OK && out[0] == 22);
    mailbox.client[0].generation = CDMB_GENERATION_MAX;
    CHECK(cdmb_begin(&mailbox, 0, input, cmx_forever(), &second) == CMX_FULL);
    struct cdmb_identity effect = {1, 10, 0};
    CHECK(cdmb_effect(&mailbox, effect, 1, &value) == CMX_OK && value == 1 &&
          mailbox.receipts[0].value == 1);
    CHECK(cdmb_acknowledge(&mailbox, effect) == CMX_OK);
    CHECK(cdmb_effect(&mailbox, effect, 1, &value) == CDMB_STALE &&
          mailbox.receipts[0].value == 1);
    CHECK(cdmb_restart(&mailbox) == CMX_OK && mailbox.session == 2);
    CHECK(cdmb_effect(&mailbox, effect, 1, &value) == CDMB_STALE &&
          cmx_invariant(&kernel));
    CHECK(cdmb_begin(&mailbox, 1, input, cmx_at(kernel.now + 5), &first) ==
          CMX_BLOCKED);
    CHECK(cdmb_receive(&mailbox, &request, 4, cmx_forever()) == CMX_OK);
    CHECK(cdmb_poll(&mailbox, 1, out, 4) == CMX_BLOCKED);
    cmx_tick(&kernel, kernel.now + 5);
    CHECK(cdmb_reply(&mailbox, first, reply) == CDMB_STALE &&
          !mailbox.client[1].has_reply);
    out[0] = 99;
    CHECK(cdmb_poll(&mailbox, 1, out, 4) == CDMB_UNKNOWN && out[0] == 99);
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

__attribute__((section(".vectors"), used)) const void *cdmb_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault, fault, fault, fault, 0, 0, 0, 0,
    fault,         fault,    0,     fault, fault};
