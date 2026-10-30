/*
 * Freestanding Cortex-M4 driver for the bit-contract reference, run under QEMU
 * with ARM semihosting (no libc). Results leave the target only through the
 * two semihosting calls below; fb_target_main never returns.
 */
#include "bits.h"
#include <limits.h>

/* ARM semihosting operation numbers and the exit reason used by `stop`. */
#define SEMIHOST_SYS_WRITE0 4u
#define SEMIHOST_SYS_EXIT_EXTENDED 0x20u
#define SEMIHOST_STOPPED_APPLICATION_EXIT 0x20026u

/* Write a NUL-terminated string to the host console. */
static void emit(const char *s)
{
    register unsigned r0 __asm__("r0") = SEMIHOST_SYS_WRITE0;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}

/* End the emulation with exit status rc; does not return. */
_Noreturn static void stop(unsigned rc)
{
    unsigned args[2] = {SEMIHOST_STOPPED_APPLICATION_EXIT, rc};
    register unsigned r0 __asm__("r0") = SEMIHOST_SYS_EXIT_EXTENDED;
    register unsigned *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
    for (;;)
    {
    }
}

#define T(x)                          \
    do                                \
    {                                 \
        if (!(x))                     \
        {                             \
            emit("FB_TARGET_FAIL\n"); \
            stop(1);                  \
        }                             \
    } while (0)

/* Called from the fault vector: report and stop with a distinct status. */
void fb_fault(void)
{
    emit("FB_TARGET_FAULT\n");
    stop(2);
}

/* Shift, rotate, field and edit operations; outputs preset to 917 must
 * survive each rejected call. */
static void check_bit_operations(void)
{
    fb_word out = 917;
    struct fb_stats stats = {0, 0, 0};
    T(fb_shift(16, 0x1234, -1, FB_LEFT, &out) == FB_RANGE && out == 917);
    T(fb_shift(16, 0x1234, 0, FB_ROTATE_LEFT, &out) == FB_OK && out == 0x1234);
    T(fb_shift(16, 0x1234, 4, FB_ROTATE_LEFT, &out) == FB_OK && out == 0x2341);
    T(fb_shift(8, 0x1234, 4, FB_RIGHT, &out) == FB_OK && out == 3);
    T(fb_extract(16, 0x1234, 4, 8, &out) == FB_OK && out == 3);
    T(fb_insert(16, 0x1234, (struct fb_edit){4, 8, 10}, &out) == FB_OK &&
      out == 0x12a4);
    T(fb_patch(8, 0xa0, (struct fb_edit){0, 4, 15}, (struct fb_edit){2, 6, 0},
               FB_SEQUENTIAL, &out) == FB_OK &&
      out == 0x83);
    out = 917;
    T(fb_patch(8, 0xa0, (struct fb_edit){0, 4, 15}, (struct fb_edit){2, 6, 0},
               FB_SIMULTANEOUS, &out) == FB_CONFLICT &&
      out == 917);
    T(fb_stats(16, 0x1234, &stats) == FB_OK && stats.population == 5 &&
      stats.width == 13 && stats.leading == 3);
}

/* Big-endian codec at an unaligned offset, and a value split across spans. */
static void check_codec(void)
{
    fb_word out = 917;
    unsigned char bytes[7] = {0};
    T(fb_encode(UINT32_C(0x12345678), FB_BIG, bytes + 3, 4) == FB_OK);
    T(fb_decode((struct fb_span){bytes + 3, 4}, FB_BIG, &out) == FB_OK &&
      out == UINT32_C(0x12345678));
    unsigned char left[] = {18, 52};
    unsigned char right[] = {86, 120};
    T(fb_decode_two((struct fb_span){left, 2}, (struct fb_span){right, 2},
                    FB_BIG, &out) == FB_OK &&
      out == UINT32_C(0x12345678));
}

/* Record and plugin-descriptor parsing against the declared size. */
static void check_wire_formats(void)
{
    const unsigned char known[] = {1, 1, 0, 8, 18, 52, 86, 120};
    const unsigned char unknown[] = {2, 255, 0, 4};
    struct fb_record record = {0, 0, 0};
    T(fb_record((struct fb_span){known, 8}, FB_REJECT_UNKNOWN, &record) ==
          FB_OK &&
      record.value == UINT32_C(0x12345678));
    T(fb_record((struct fb_span){unknown, 4}, FB_SKIP_UNKNOWN, &record) ==
          FB_SKIPPED &&
      record.consumed == 4);

    const unsigned char v1[] = {0, 1, 0, 8, 0, 0, 0, 1};
    const unsigned char v2[] = {0, 2, 0, 12, 0, 0, 0, 1, 0, 0, 4, 0};
    struct fb_plugin p = {0, 0, 0, 0};
    T(fb_negotiate((struct fb_span){v1, 8}, &p) == FB_OK && p.max_items == 0);
    T(fb_negotiate((struct fb_span){v2, 12}, &p) == FB_OK &&
      p.max_items == 1024);
    /* A truncated view of v2 is refused and leaves the last result alone. */
    T(fb_negotiate((struct fb_span){v2, 8}, &p) == FB_SHORT &&
      p.max_items == 1024);
}

/* Teaching register: ordinary low nibble, write-1-to-clear high nibble, and
 * the read-to-clear variant. */
static void check_register(void)
{
    struct fb_register r;
    T(fb_register_init(&r, 0xa5, 0) == FB_OK);
    unsigned old = fb_register_read(&r);
    fb_register_write(&r, (old & 240) | 3);
    T(r.state == 3);
    T(fb_register_init(&r, 0xa5, 1) == FB_OK);
    T(fb_register_update(&r, 3, 0) == FB_OK && r.state == 0xa3 && r.reads == 0);
    T(fb_register_read(&r) == 0xa3 && r.state == 3);
}

void fb_target_main(void)
{
    check_bit_operations();
    check_codec();
    check_wire_formats();
    check_register();
    emit(
        "FB_TARGET_PASS shifts fields codec prior_integer register_models version_bounds\n");
    stop(0);
}
