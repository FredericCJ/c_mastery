#include "checkpoint_firmware.h"
#include "clock.h"
#include "handoff.h"
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))

void xk_print(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

static void hex(uint32_t x)
{
    char b[10];
    for (unsigned i = 0; i < 8; ++i)
        b[i] = "0123456789abcdef"[(x >> (28 - 4 * i)) & 15];
    b[8] = ' ';
    b[9] = 0;
    xk_print(b);
}

__attribute__((noreturn)) static void finish(unsigned code)
{
    unsigned args[2] = {0x20026, code};
    register unsigned r0 __asm__("r0") = 0x20;
    register unsigned *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
    }
}

#define CHECK(x)                                  \
    do                                            \
    {                                             \
        if (!(x))                                 \
        {                                         \
            xk_print("XK_TARGET_CHECK " #x "\n"); \
            finish(1);                            \
        }                                         \
    } while (0)

void xf_fault(void)
{
    xk_print("XK_FAULT\n");
    finish(2);
}

static uint32_t now(void)
{
    return UINT32_MAX - REG(0x40001004);
}

static uint32_t hi(void *p)
{
    (void)p;
    return 0;
}

static uint32_t lo(void *p)
{
    (void)p;
    return now();
}

static uint64_t guard(void *p)
{
    (void)p;
    return now();
}
static struct xi_queue queue;
static struct cpf_clock software_clock;
static volatile unsigned done, current;
static unsigned drops;
static uint32_t begins[7], ends[7], samples[7], entries[7], posted[7];

void xk_timer(void)
{
    entries[current] = now();
    REG(0x40000000) = 0;
    REG(0x4000000c) = 1;
    __asm__ volatile("dsb sy" ::: "memory");
    uint64_t software = 999;
    if (current == 0)
        CHECK(!cpf_clock_read(&software_clock, 2, &software) &&
              software == 999);
    else
        CHECK(cpf_clock_read(&software_clock, 2, &software) &&
              software == UINT64_C(0x800000000));
    struct xk_snapshot snapshot;
    CHECK(xk_snapshot((struct xk_io){0, hi, lo, guard},
                      (struct xk_policy){32, 1, 2, 2001, 2000},
                      &snapshot) == XK_OK);
    CHECK(snapshot.begin <= snapshot.value && snapshot.value <= snapshot.end &&
          snapshot.end < 1000000);
    begins[current] = (uint32_t)snapshot.begin;
    ends[current] = (uint32_t)snapshot.end;
    samples[current] = (uint32_t)snapshot.value;
    struct xi_record record = {.sequence = current,
                               .timestamp = (uint32_t)snapshot.value};
    int rc = xi_push(&queue, &record);
    CHECK(rc == (current >= 3 && current < 6 ? XI_FULL : XI_OK));
    if (rc == XI_FULL)
        ++drops;
    done = 1;
}

int xk_integration(void);

void xf_target_main(void)
{
    struct xk_calibration wide = {0, 0, 0, 1, 0, 1};
    struct xk_interval edge;
    CHECK(xk_calibrate(&wide, UINT64_MAX, UINT64_MAX, &edge) == XK_OK &&
          edge.lower == UINT64_MAX && edge.upper == UINT64_MAX);
    wide.anchor_upper_ns = 1;
    CHECK(xk_calibrate(&wide, UINT64_MAX, UINT64_MAX, &edge) == XK_RANGE);
    wide.anchor_upper_ns = 0;
    wide.quantum_ns = UINT32_MAX;
    CHECK(xk_calibrate(&wide, UINT32_MAX, UINT32_MAX, &edge) == XK_OK &&
          edge.lower == UINT64_C(18446744065119617025));
    wide.rate_error_ppm = 1;
    CHECK(xk_calibrate(&wide, UINT32_MAX, UINT32_MAX, &edge) == XK_RANGE);
    xk_print("XK_ARITHMETIC_PASS large_division_boundaries=4\n");
    unsigned touching = 99;
    CHECK(xk_order((struct xk_interval){1, 2, 1}, (struct xk_interval){2, 3, 1},
                   &touching) == XK_OK &&
          touching == XK_AMBIGUOUS);
    if (xk_integration())
        finish(1);
    CHECK(cpf_clock_init(&software_clock));
    CHECK(cpf_clock_write(&software_clock, UINT64_C(0x7ffffffff)));
    CHECK(xi_init(&queue, 4) == XI_OK);
    REG(0x40001000) = 0;
    REG(0x40001008) = UINT32_MAX;
    REG(0x40001000) = 1;
    REG(0xe000e100) = 1u << 8;
    *(volatile uint8_t *)(uintptr_t)(0xe000e400 + 8) = 0x60;
    __asm__ volatile("cpsie i" ::: "memory");
    struct xk_sequence sequence = {0};
    struct xi_record record;
    for (current = 0; current < 7; ++current)
    {
        if (current == 0)
        {
            atomic_store(&software_clock.sequence, 3);
            atomic_store(&software_clock.low, 0);
        }
        if (current == 1)
        {
            atomic_store(&software_clock.high, 8);
            atomic_store(&software_clock.sequence, 4);
        }
        if (current == 6)
        {
            for (unsigned i = 0; i < 3; ++i)
            {
                CHECK(xi_pop(&queue, &record) == XI_OK && record.sequence == i);
                CHECK(xk_sequence(&sequence, 1, record.sequence, 16) == XK_OK);
            }
        }
        done = 0;
        posted[current] = now();
        REG(0x40000000) = 0;
        REG(0x40000008) = 1000;
        REG(0x40000000) = 9;
        while (!done)
            __asm__ volatile("nop" ::: "memory");
        struct xk_calibration calibration = {0, 0, 0, 40, 0, 1};
        struct xk_interval interval;
        CHECK(xk_calibrate(&calibration, begins[current], ends[current],
                           &interval) == XK_OK);
        unsigned relation = 99;
        CHECK(xk_order(interval, interval, &relation) == XK_OK &&
              relation == XK_AMBIGUOUS);
        xk_print("XK_CAPTURE ");
        hex(current);
        hex(posted[current]);
        hex(entries[current]);
        hex(begins[current]);
        hex(samples[current]);
        hex(ends[current]);
        hex((uint32_t)interval.lower);
        hex((uint32_t)interval.upper);
        xk_print("\n");
    }
    CHECK(drops == 3 && xi_pop(&queue, &record) == XI_OK &&
          record.sequence == 6);
    CHECK(xk_sequence(&sequence, 1, record.sequence, 16) == XK_GAP &&
          sequence.missing == 3);
    xk_print("XK_TARGET_PASS actual_irq=7 queued=4 dropped=3 "
             "odd_snapshot_refused=1\n");
    finish(0);
}
