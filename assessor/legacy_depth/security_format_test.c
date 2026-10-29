#include <stdio.h>
#include <string.h>
#include <stddef.h>

int teaching_render(const char *, int *, char *, size_t);

#define REQUIRE(e)                                                            \
    do                                                                        \
    {                                                                         \
        if (!(e))                                                             \
        {                                                                     \
            fprintf(stderr, "FORMAT_AUTHORITY_ASSERT:%d:%s\n", __LINE__, #e); \
            return 1;                                                         \
        }                                                                     \
    } while (0)

int main(void)
{
    char out[8] = "seed";
    int marker = 77;
    int result = teaching_render("%n", &marker, out, sizeof out);
#if EXPECT_FLAW
    REQUIRE(result == 0 && marker == 0 && out[0] == '\0');
    puts(
        "flawed: return=0 marker=0 output=empty; synthetic integrity violation");
#else
    REQUIRE(result == 2 && marker == 77 && strcmp(out, "%n") == 0);
    for (size_t capacity = 1; capacity <= 3; ++capacity)
    {
        memset(out, 'x', sizeof out);
        result = teaching_render("%n", &marker, out, capacity);
        REQUIRE(result == 2 && marker == 77);
        REQUIRE(out[capacity - 1] == '\0' && out[capacity] == 'x');
        REQUIRE(capacity < 2 || out[0] == '%');
        REQUIRE(capacity < 3 || out[1] == 'n');
    }
    puts("repaired: return=2 marker=77 output=%n; capacities 1,2,3 verified");
#endif
    return 0;
}
