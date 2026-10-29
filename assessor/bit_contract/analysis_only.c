#include <stdint.h>

struct __attribute__((packed)) FbOverlay
{
    unsigned char tag;
    uint32_t value;
    unsigned short count;
};

/* Compile only: padding/packing does not establish effective type, lifetime,
 * wire version/length or alignment legality for a forged overlay. */
uint32_t fb_bad_overlay(const unsigned char *bytes)
{
    return ((const struct FbOverlay *)bytes)->value;
}

unsigned fb_bad_clz(unsigned value)
{
    return (unsigned)__builtin_clz(value);
}

unsigned fb_bad_rotate(unsigned value, unsigned count)
{
    return (value << count) | (value >> (32 - count));
}
