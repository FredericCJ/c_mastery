#include <stddef.h>
#include <string.h>
int main(void) {
    unsigned char a[19], b[19];
    for (size_t n = 0; n <= sizeof a; ++n) {
        for (size_t i = 0; i < sizeof a; ++i) { a[i] = 0x91; b[i] = (unsigned char)i; }
        if (memset(a, 0x1a5, n) != a) return 1;
        for (size_t i = 0; i < sizeof a; ++i)
            if (a[i] != (i < n ? 0xa5 : 0x91)) return 2;
        if (memcpy(a, b, n) != a) return 3;
        for (size_t i = 0; i < sizeof a; ++i)
            if (a[i] != (i < n ? i : 0x91)) return 4;
    }
    return 0;
}
