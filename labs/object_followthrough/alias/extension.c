/*
 * Compiler-extension experiment, not ISO C: reads a float's bits through a
 * GCC/Clang may_alias type and compares them with a character-type copy.
 * Assumes binary32 float; exit status 2 means that assumption failed, 41 that
 * the two readings differ, 0 that they agree (marker OS_EXTENSION_PASS).
 */
#include <stdint.h>
#include <stdio.h>
#include <float.h>

/* may_alias exempts accesses through this type from strict-aliasing rules. */
typedef uint32_t __attribute__((may_alias)) alias_word;

int main(void)
{
    _Static_assert(sizeof(float) == sizeof(uint32_t),
                   "Selected binary32 representation comparison only");
    _Static_assert(_Alignof(float) >= _Alignof(uint32_t),
                   "Selected alias alignment");
    const uint32_t binary32_one = UINT32_C(0x3f800000);
    float f = 1.0f;
    uint32_t bytes = 0;
    unsigned char *to = (unsigned char *)&bytes;
    const unsigned char *from = (const unsigned char *)&f;
    for (unsigned i = 0; i < sizeof f; ++i)
    {
        to[i] = from[i];
    }
    if (FLT_RADIX != 2 || FLT_MANT_DIG != 24 || bytes != binary32_one)
    {
        return 2;
    }
    alias_word *extension = (alias_word *)(void *)&f;
    if (*extension != bytes)
    {
        return 41;
    }
    puts(
        "OS_EXTENSION_PASS:selected GCC/Clang may_alias read equals character copy");
    return 0;
}
