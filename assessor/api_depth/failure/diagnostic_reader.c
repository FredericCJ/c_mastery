#include "public_error.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    FILE *f = fopen(argv[1], "rb");
    if (!f)
        return 2;
    unsigned char b[17];
    size_t n = fread(b, 1, 17, f);
    int failed = ferror(f);
    if (fclose(f))
        failed = 1;
    if (failed || n != 16)
    {
        fprintf(stderr, "DIAGNOSTIC-REJECT:length\n");
        return 7;
    }
    uint32_t code = 0, op = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        code |= (uint32_t)b[i] << (8 * i);
        op |= (uint32_t)b[i + 4] << (8 * i);
    }
    unsigned backend, operation, stage;
    int cause;
    if (ae_decode(b + 8, 8, &backend, &cause, &operation, &stage) != AE_OK ||
        op != operation || code != AE_TIMEOUT || backend != 1 || cause != 104 ||
        stage != 2)
    {
        fprintf(stderr, "DIAGNOSTIC-REJECT:contract\n");
        return 7;
    }
    printf("DIAGNOSTIC-PROCESS-PASS:code=%u;op=%u;cause=%d;bytes=16;\n",
           (unsigned)code, (unsigned)op, cause);
    return 0;
}
