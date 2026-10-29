#include "authority_boundary.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    unsigned char bytes[17];
    size_t n = fread(bytes, 1, sizeof bytes, stdin);
    if (ferror(stdin))
        return 2;
    struct ow_tenant_arena a;
    for (unsigned i = 0; i < 16; i++)
        a.data[i] = (int32_t)(90 + i);
    struct oaf_root root;
    struct oaf_view full, parent;
    if (oaf_init(&root, &a, 1, 3) != OAF_OK ||
        oaf_view(&root, &full) != OAF_OK ||
        oaf_subview(&full, 2, 4, 1, &parent) != OAF_OK)
        return 2;
    int32_t out[8];
    for (unsigned i = 0; i < 8; i++)
        out[i] = -9;
    enum oaf_status s = !strcmp(argv[1], "read")
                            ? oaf_wire_read(&parent, bytes, n, out)
                            : oaf_wire_write(&parent, bytes, n, 7);
    printf("%u", (unsigned)s);
    for (unsigned i = 0; i < 16; i++)
        printf(",%" PRId32, a.data[i]);
    for (unsigned i = 0; i < 8; i++)
        printf(",%" PRId32, out[i]);
    putchar('\n');
    return 0;
}
