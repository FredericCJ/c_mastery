/* Original bounded format-string teaching target. No external input/addresses. */
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *input = "abc%n";
    int sentinel = 91;
    char out[32];
    if (argc != 2)
    {
        return 2;
    }
    if (strcmp(argv[1], "flawed") == 0)
    {
        snprintf(out, sizeof out, input, &sentinel);
    }
    else if (strcmp(argv[1], "repaired") == 0)
    {
        snprintf(out, sizeof out, "%s", input);
    }
    else
    {
        return 2;
    }
    printf("sentinel=%d output=%s\n", sentinel, out);
    return sentinel == (strcmp(argv[1], "flawed") == 0 ? 3 : 91) ? 0 : 1;
}
