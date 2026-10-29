#include "coverage.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CV_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

int main(int argc, char **argv)
{
    int weak = argc == 2 && strcmp(argv[1], "weak") == 0;
    struct cv_cv c = {0};
    CHECK(cv_push(&c, 5) == 1 && cv_push(&c, 9) == 1);
    CHECK(cv_allow(&c, 2, 0) == 0);
    c.live = 3;
    cv_cleanup(&c);
    if (weak)
    {
        puts("CV_WEAK_PASS:return-only-two-pushes");
        return 0;
    }
    CHECK(c.rejection_count == 1);
    CHECK(c.live == 0);
    CHECK(cv_push(&c, 11) == 1 && cv_push(&c, 13) == 1 && cv_push(&c, 15) == 0);
    CHECK(c.count == 4 && c.queue[0] == 5 && c.queue[1] == 9 &&
          c.queue[2] == 11 && c.queue[3] == 13);
    CHECK(cv_allow(&c, 4, 1) == 1 && cv_allow(&c, 5, 1) == 0);
    CHECK(cv_decision(1, 1) == 1 && cv_decision(0, 1) == 0 &&
          cv_decision(1, 0) == 0);
    for (int a = 0; a < 2; a++)
    {
        for (int b = 0; b < 2; b++)
        {
            for (int d = 0; d < 2; d++)
            {
                CHECK(cv_changed(a, b, d) == ((a + b == 2) || d));
            }
        }
    }
    puts(
        "CV_PASS:capacity,authorization,rejection-effect,cleanup,truth-relations");
    return 0;
}
