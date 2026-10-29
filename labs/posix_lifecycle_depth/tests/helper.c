#include <signal.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "zero"))
    {
        return 0;
    }
    if (!strcmp(argv[1], "127"))
    {
        return 127;
    }
    if (!strcmp(argv[1], "signal"))
    {
        raise(SIGTERM);
        return 3;
    }
    return 4;
}
