/* Comparison probe only: never part of the portable C17 implementation. */
#define FIRST(x, ...) x __VA_OPT__(+__VA_ARGS__)
_Static_assert(FIRST(2) == 2, "empty tokens");
_Static_assert(FIRST(2, 3) == 5, "nonempty tokens");

int probe(void)
{
    return FIRST(2, 3);
}
