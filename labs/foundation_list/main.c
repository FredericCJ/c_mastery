#include "list.h"
#include <inttypes.h>
#include <stdio.h>

/* Command-line adapter: prints one line and exits with the status number.
 *
 * Success (FT_OK or FT_EMPTY) prints "<name> count=<n> sum=<s>" and exits 0.
 * Any other status prints only its name and exits with the enumerator value.
 *
 * The two application translation units are this effectful adapter and the
 * learner's list.c. No scanf/strtol implements the assessed parser. */

/* Copies up to FT_CAPACITY + 1 arguments into tokens and returns the count.
 * In fail-fast mode the first token beyond capacity must reject, so later
 * arguments cannot alter the first error and need not be passed on. The
 * pointer values are copied element by element (a permitted qualification
 * conversion) rather than casting a char ** to const char *const *. */
static size_t collect_tokens(int argc, char **argv,
                             const char *tokens[FT_CAPACITY + 1])
{
    size_t count = argc > 1 ? (size_t)(argc - 1) : 0;

    if (count > FT_CAPACITY + 1)
    {
        count = FT_CAPACITY + 1;
    }
    for (size_t i = 0; i < count; ++i)
    {
        tokens[i] = argv[i + 1];
    }
    return count;
}

int main(int argc, char **argv)
{
    /* Sentinel values prove that a failing call leaves the result alone. */
    struct ft_result result = {91, 92};
    struct ft_errors errors;
    const char *tokens[FT_CAPACITY + 1];
    const size_t count = collect_tokens(argc, argv, tokens);
    const enum ft_status status = ft_tokens(tokens, count, 0, &result, &errors);

    if (status == FT_OK || status == FT_EMPTY)
    {
        printf("%s count=%zu sum=%" PRId32 "\n", ft_name(status), result.count,
               result.sum);
        return 0;
    }
    printf("%s\n", ft_name(status));
    return (int)status;
}
