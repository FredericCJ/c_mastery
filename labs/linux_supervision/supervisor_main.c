#define _POSIX_C_SOURCE 200809L
#include "supervision.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    static const struct {
        const char *name;
        int (*run)(const char *variant);
    } cases[] = {
        {"namespace", supervise_namespace},
        {"pids", supervise_pids},
        {"shutdown", supervise_shutdown},
        {"network", supervise_network},
        {"telemetry", supervise_telemetry}
    };
    struct stat marker;
    if (argc != 3) {
        fputs("usage: supervise CASE VARIANT\n", stderr);
        return 2;
    }
    if (getpid() != 1 || getuid() != 1000 || geteuid() != 1000 ||
        lstat("/run/linux-supervision-guest", &marker) || !S_ISREG(marker.st_mode)) {
        fputs("start this program through the Linux supervision guest fixture\n", stderr);
        return 1;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
        if (!strcmp(argv[1], cases[i].name))
            return cases[i].run(argv[2]);
    return 2;
}
