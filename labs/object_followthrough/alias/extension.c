#include <stdint.h>
#include <stdio.h>
#include <float.h>
typedef uint32_t __attribute__((may_alias)) alias_word;

int main(void)
{
    _Static_assert(sizeof(float) == sizeof(uint32_t),
                   "Selected binary32 representation comparison only");
    _Static_assert(_Alignof(float) >= _Alignof(uint32_t),
                   "Selected alias alignment");
    float f = 1.0f;
    uint32_t bytes = 0;
    unsigned char *to = (unsigned char *)&bytes;
    const unsigned char *from = (const unsigned char *)&f;
    for (unsigned i = 0; i < sizeof f; ++i)
        to[i] = from[i];
    if (FLT_RADIX != 2 || FLT_MANT_DIG != 24 || bytes != UINT32_C(0x3f800000))
        return 2;
    alias_word *extension = (alias_word *)(void *)&f;
    if (*extension != bytes)
        return 41;
    puts(
        "OS_EXTENSION_PASS:selected GCC/Clang may_alias read equals character copy");
    return 0;
}
