#include "list.h"
#include <inttypes.h>
#include <stdio.h>
/* The two application translation units are this effectful adapter and the
 * learner's list.c. No scanf/strtol implements the assessed parser. */
int main(int argc, char **argv) {
  FtResult result = {91, 92};
  FtErrors errors;
  const char *tokens[FT_CAPACITY + 1];
  size_t count = argc > 1 ? (size_t)(argc - 1) : 0;
  /* In fail-fast mode the first token beyond capacity must reject, so later
   * arguments cannot alter the first error. Copy pointer values with their
   * permitted qualification conversion, rather than casting char **. */
  if (count > FT_CAPACITY + 1)
    count = FT_CAPACITY + 1;
  for (size_t i = 0; i < count; ++i)
    tokens[i] = argv[i + 1];
  FtStatus s = ft_tokens(tokens, count, 0, &result, &errors);
  if (s == FT_OK || s == FT_EMPTY) {
    printf("%s count=%zu sum=%" PRId32 "\n", ft_name(s), result.count,
           result.sum);
    return 0;
  }
  printf("%s\n", ft_name(s));
  return (int)s;
}
