#include "hex.h"
#include <inttypes.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    char text[4097];
    struct sf_image image;
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '2' || argv[1][1])
        return 2;
    size_t n = fread(text, 1, sizeof text, stdin);
    enum sf_status r =
        sf_hex(text, n, (enum sf_mode)(argv[1][0] - '0'), &image);
    printf("%u", (unsigned)r);
    if (r == SF_OK)
    {
        printf(" %zu %u %" PRIu32, image.count, image.entry_kind, image.entry);
        for (size_t i = 0; i < image.count; i++)
            printf(" %" PRIu32 ":%u", image.bytes[i].address,
                   (unsigned)image.bytes[i].value);
    }
    putchar('\n');
    return 0;
}
