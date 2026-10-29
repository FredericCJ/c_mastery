#include "service.h"
static volatile uint32_t initialized = UINT32_C(0x97531ace), zeroed;
static struct cs_service service;

static void semihost(unsigned op, const void *arg)
{
    register unsigned r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

static void stop(unsigned error)
{
    semihost(4, error ? "CAP_TARGET_FAIL\n" : "CAP_TARGET_PASS\n");
    unsigned args[2] = {0x20026, error};
    semihost(0x20, args);
    for (;;)
    {
    }
}

void cp_fault(void)
{
    stop(2);
}

void cp_pendsv(void)
{
    stop(3);
}

#define REQUIRE(x)   \
    do               \
    {                \
        if (!(x))    \
            stop(1); \
    } while (0)

static struct cs_io output(void *p, const unsigned char *b, size_t n)
{
    size_t *count = p;
    REQUIRE(n > 0 && b[0] == 0xa5);
    *count += n;
    return (struct cs_io){n, CS_IO_PROGRESS, 0};
}

void cp_target_main(void)
{
    REQUIRE(initialized == UINT32_C(0x97531ace) && !zeroed);
    REQUIRE(cs_init(&service, 3, 100) == CS_OK);
    struct cp_message message = {258, 32, {0}};
    for (unsigned i = 0; i < 32; ++i)
    {
        message.payload[i] = (unsigned char)(0xa5u + i);
    }
    unsigned char wire[42];
    size_t n, used;
    REQUIRE(cp_encode(&message, wire, 42, &n) == CP_OK && n == 42);
    for (size_t split = 0; split <= 42; ++split)
    {
        REQUIRE(cs_init(&service, 3, 100) == CS_OK);
        enum cs_status status = cs_feed(&service, 0, wire, split, &used, 1, 0);
        REQUIRE(used == split && status == (split == 42 ? CS_FRAME : CS_MORE));
        if (split < 42)
        {
            REQUIRE(cs_feed(&service, 0, wire + split, 42 - split, &used, 1,
                            0) == CS_FRAME);
        }
        REQUIRE(cs_begin(&service) == CS_OK);
        size_t count = 0;
        REQUIRE(cs_step(&service, output, &count) == CS_DONE && count == 42 &&
                cs_conserved(&service));
    }
    REQUIRE(cs_init(&service, 1, 100) == CS_OK &&
            cs_offer(&service, &message, 0, 0, 0) == CS_DENIED &&
            !service.accepted);
    stop(0);
}
