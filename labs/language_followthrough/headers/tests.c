/*
 * Driver for the header-definitions lab. It links against two independent
 * consumers of headers.h and exactly one external definition.
 *
 * Usage: tests SELECTOR
 *   0  helper identity and hh_read contract
 *   1  per-translation-unit state
 * Any other argument, or none, exits with status 2. A failed requirement
 * prints "HH_ASSERT:<expression>" to stderr and exits with status 41.
 */
#include <headers.h>
#include <limits.h>
#include <stdio.h>

/* Entry points defined by consumer_a.c and consumer_b.c. */
hh_fn hh_a_address(void);
hh_fn hh_b_address(void);
unsigned *hh_a_state(void);
unsigned *hh_b_state(void);
unsigned hh_a_call(unsigned);
unsigned hh_b_call(unsigned);

enum
{
    EXIT_REQUIREMENT_FAILED = 41,
    EXIT_BAD_USAGE = 2
};

enum selector
{
    SELECTOR_DEFINITIONS = 0,
    SELECTOR_LOCAL_STATE = 1,
    SELECTOR_INVALID
};

#define REQUIRE(condition)                                 \
    do                                                     \
    {                                                      \
        if (!(condition))                                  \
        {                                                  \
            fprintf(stderr, "HH_ASSERT:%s\n", #condition); \
            return EXIT_REQUIREMENT_FAILED;                \
        }                                                  \
    } while (0)

/*
 * Accepts exactly one character, '0' or '1'. Anything else (NULL, empty, a
 * sign, trailing text) is SELECTOR_INVALID rather than a guessed number.
 */
static enum selector parse_selector(int argc, char **argv)
{
    if (argc != 2 || argv[1] == NULL || argv[1][0] == '\0' ||
        argv[1][1] != '\0')
    {
        return SELECTOR_INVALID;
    }
    if (argv[1][0] == '0')
    {
        return SELECTOR_DEFINITIONS;
    }
    if (argv[1][0] == '1')
    {
        return SELECTOR_LOCAL_STATE;
    }
    return SELECTOR_INVALID;
}

/* Helper identity, wraparound arithmetic and the hh_read failure contract. */
static int check_definitions(void)
{
    /* Keep the names a and out: the mutation campaign matches the checks. */
    int a[4] = {10, 20, 30, 40};
    int out = 77;

    REQUIRE(hh_a_address() != hh_b_address());
    REQUIRE(hh_a_address()(7) == 22 && hh_b_call(7) == 22);
    REQUIRE(hh_a_call(UINT_MAX) == UINT_MAX - 1u);
    REQUIRE(hh_read(a, 4, 3, &out) == 0 && out == 40);

    /* A rejected read leaves the live output object unchanged. */
    REQUIRE(hh_read(a, 4, 4, &out) == -1 && out == 40);
    REQUIRE(hh_read(NULL, 0, 0, &out) == -1 && out == 40);
    REQUIRE(hh_read(a, 4, 0, NULL) == -1);
    puts("HH_DEFINITIONS_PASS");
    return 0;
}

/* Each translation unit gets its own header-local static counter. */
static int check_local_state(void)
{
    unsigned *a = hh_a_state();
    unsigned *b = hh_b_state();

    REQUIRE(hh_a_call(7) == 22);
    REQUIRE(a != b && *a == 0 && *b == 0);
    *a = 7;
    REQUIRE(*b == 0);
    *b = 9;
    REQUIRE(hh_a_state() == a && *a == 7 && hh_b_state() == b && *b == 9);
    puts("HH_LOCAL_STATE_PASS");
    return 0;
}

int main(int argc, char **argv)
{
    switch (parse_selector(argc, argv))
    {
        case SELECTOR_DEFINITIONS:
            return check_definitions();
        case SELECTOR_LOCAL_STATE:
            return check_local_state();
        case SELECTOR_INVALID:
        default:
            return EXIT_BAD_USAGE;
    }
}
