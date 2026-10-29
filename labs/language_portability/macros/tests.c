#include "macros.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                  \
    do                                            \
    {                                             \
        if (!(x))                                 \
        {                                         \
            fprintf(stderr, "LP_CHECK:%s\n", #x); \
            return 1;                             \
        }                                         \
    } while (0)

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "prescan"))
    {
        CHECK(lm_forward() == 7);
        CHECK(lm_compound() == 3);
        puts("LP prescan validated");
    }
    else if (!strcmp(argv[1], "stringify"))
    {
        CHECK(!strcmp(lm_direct(), "VALUE"));
        CHECK(!strcmp(lm_expanded(), "7"));
        CHECK(!strcmp(lm_escaped(), "alpha \"q\\\"x\" + beta"));
        puts("LP stringify validated");
    }
    else if (!strcmp(argv[1], "paste"))
    {
        CHECK(lm_paste(0) == 11);
        CHECK(lm_paste(1) == 22);
        puts("LP paste validated");
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        printf("%d %d %d %d\n%s\n%s\n%s\n", lm_forward(), lm_compound(),
               lm_paste(0), lm_paste(1), lm_direct(), lm_expanded(),
               lm_escaped());
    }
    else if (!strcmp(argv[1], "metrics"))
    {
        puts(
            "{\"allocations\":0,\"macro_operations\":6,\"evaluated_forward_inputs\":1}");
    }
    else
        return 2;
    return 0;
}
