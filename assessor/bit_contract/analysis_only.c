/* CM-SPECIMEN: forged struct overlay on a byte buffer, clz(0), and a rotate
 * by the full word width, each in an unguarded function.
 *
 * Compile only; never linked or called. Packing a struct removes padding but
 * does not establish effective type, lifetime, wire version or length, or
 * alignment legality for an overlay forged onto bytes (SEI CERT EXP36-C,
 * EXP39-C); __builtin_clz(0) and a shift by the operand width are undefined
 * (CERT INT34-C). The guarded versions are in bits.c and intrinsics.c. */
#include <stdint.h>

struct __attribute__((packed)) fb_overlay
{
    unsigned char tag;
    uint32_t value;
    unsigned short count;
};

uint32_t fb_bad_overlay(const unsigned char *bytes)
{
    return ((const struct fb_overlay *)bytes)->value;
}

unsigned fb_bad_clz(unsigned value)
{
    return (unsigned)__builtin_clz(value);
}

unsigned fb_bad_rotate(unsigned value, unsigned count)
{
    return (value << count) | (value >> (32 - count));
}
