#include "verifier.h"
#include <stdio.h>
static struct vv_work work;

int main(void)
{
    unsigned count = 0, accepted = 0;
    unsigned char header[3], image[1100];
    while (fread(header, 1, 3, stdin) == 3)
    {
        size_t n = (size_t)header[0] | ((size_t)header[1] << 8);
        struct cvm_program p;
        struct vv_report r;
        if (!n || n > sizeof image || header[2] > 1 ||
            fread(image, 1, n, stdin) != n ||
            cvm_decode(image, n, &p) != CVM_OK)
            return 2;
        int got = vv_verify(&p, 0, 1, &work, &r);
        if (got != (header[2] ? VV_OK : VV_INITIALIZED) ||
            r.processed > 9 * r.decoded || r.edges > 2 * r.processed ||
            r.max_queue > r.decoded || r.local_changes > 8 * r.decoded)
        {
            printf("VV_ORACLE_FAIL case=%u status=%d expected=%u pc=%u\n",
                   count, got, (unsigned)header[2], r.pc);
            return 1;
        }
        accepted += header[2];
        if (++count > 12288)
            return 2;
    }
    if (!feof(stdin) || count != 12288)
        return 2;
    printf("VV_ORACLE_PASS cases=%u accepted=%u concrete_path_sets\n", count,
           accepted);
    return 0;
}
