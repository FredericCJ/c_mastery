/* Public harness; uses the learner implementation of the compiler mechanism. */
#include "core.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    struct cc_ins ir[128];
    size_t n = 0;
    char output[4096];
    int ok;
    if (argc != 3)
    {
        return 2;
    }
    if (!cc_parse(argv[2], strlen(argv[2]), ir, 128, &n))
    {
        return 3;
    }
    if (strcmp(argv[1], "x86") == 0)
    {
        ok = cc_x86(ir, n, output, sizeof output);
    }
    else if (strcmp(argv[1], "thumb") == 0)
    {
        ok = cc_thumb(ir, n, output, sizeof output);
    }
    else
    {
        return 2;
    }
    if (!ok)
    {
        return 4;
    }
    return fputs(output, stdout) < 0 ? 5 : 0;
}
