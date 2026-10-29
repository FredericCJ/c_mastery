#include "checks.h"
#include <stdlib.h>
#include <stdio.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (!one_case(data, size))
    {
        fputs("TF_FUZZ_SEMANTIC:grammar-or-publication\n", stderr);
        abort();
    }
    return 0;
}
