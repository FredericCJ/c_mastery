#include "public_error.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    struct ae_public p;
    unsigned char b[16] = {0};
    if (ae_capture(1, 104, 9, 2, &p, b + 8) != AE_OK)
        return 3;
    for (unsigned i = 0; i < 4; ++i)
    {
        b[i] = (unsigned char)((p.code >> (i * 8)) & 255u);
        b[i + 4] = (unsigned char)((p.operation >> (i * 8)) & 255u);
    }
    FILE *f = fopen(argv[1], "wb");
    if (!f)
        return 4;
    int result = fwrite(b, 1, 16, f) == 16 ? 0 : 5;
    if (fclose(f))
        result = 6;
    return result;
}
