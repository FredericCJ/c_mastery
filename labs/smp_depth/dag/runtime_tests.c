#include <stddef.h>
#include <stdio.h>
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);

int main(void)
{
    unsigned char source[257], destination[259];
    for (size_t n = 0; n <= 257; ++n)
    {
        for (size_t i = 0; i < 257; ++i)
            source[i] = (unsigned char)(i * 17);
        for (size_t i = 0; i < 259; ++i)
            destination[i] = 91;
        if (memcpy(destination + 1, source, n) != destination + 1)
            return 1;
        for (size_t i = 0; i < 259; ++i)
            if (destination[i] != (i >= 1 && i <= n ? source[i - 1] : 91))
                return 1;
        if (memset(destination + 1, 0x123, n) != destination + 1)
            return 1;
        for (size_t i = 0; i < 259; ++i)
            if (destination[i] != (i >= 1 && i <= n ? 0x23 : 91))
                return 1;
    }
    puts(
        "DG_RUNTIME_PASS:258-lengths-return-values-guards-and-byte-conversion");
    return 0;
}
