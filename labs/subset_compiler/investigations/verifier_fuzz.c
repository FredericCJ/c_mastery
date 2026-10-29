#include "extension.h"
#include <string.h>

/* Independent of the text parser and byte decoder: directly construct bounded
   typed CFG candidates. This envelope restricts one function, not the verifier. */
int LLVMFuzzerTestOneInput(const uint8_t *d, size_t n)
{
    struct ce_program p;
    unsigned h[CE_CODE], fuel = 1024;
    uint32_t out = 91;
    if (n < 3 || n > 4099)
        return 0;
    memset(&p, 0, sizeof p);
    p.functions = 1;
    p.count = (unsigned)((n - 3) / 8);
    if (!p.count || p.count > CE_CODE)
        return 0;
    p.fn[0].locals = 1 + d[0] % CE_LOCALS;
    p.fn[0].end = p.count;
    strcpy(p.fn[0].name, "main");
    p.fn[0].types[0] = CE_U32;
    for (unsigned i = 1; i < p.fn[0].locals; i++)
        p.fn[0].types[i] = 1 + (d[1] + i * d[2]) % 3;
    for (unsigned i = 0; i < p.count; i++)
    {
        size_t k = 3 + 8 * i;
        for (unsigned j = 0; j < 4; j++)
        {
            p.code[i].op |= (unsigned)d[k + j] << (8 * j);
            p.code[i].arg |= (unsigned)d[k + 4 + j] << (8 * j);
        }
    }
    if (ce_verify(&p, h) && !ce_run(&p, 17, &fuel, &out) && out != 91)
        __builtin_trap();
    return 0;
}
