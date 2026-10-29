#define _GNU_SOURCE
#include "test.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
extern char **environ;
static int image_private = 7;

int main(int argc, char **argv)
{
    char cwd[4096];
    struct rlimit lim;
    size_t count = 0;
    CHECK(argc == 3 && strcmp(argv[1], "") == 0 &&
          strcmp(argv[2], "with spaces") == 0);
    if (!getenv("PC_CONFIG"))
    {
        puts("PC_MISSING_CONFIG");
        return 23;
    }
    CHECK(!strcmp(getenv("PC_CONFIG"), "synthetic=alpha"));
    CHECK(getenv("PC_SECOND") && !strcmp(getenv("PC_SECOND"), "beta"));
    while (environ[count])
    {
        ++count;
    }
    CHECK(count == 2 && !getenv("PATH"));
    CHECK(getcwd(cwd, sizeof cwd) && getrlimit(RLIMIT_NOFILE, &lim) == 0);
    CHECK(image_private ==
          7); /* A new image, not the old process's modified global. */
    printf(
        "PC_HELPER pid=%ld euid=%lu egid=%lu cwd=%s soft=%llu hard=%llu argc=%d env=%zu private=%d\n",
        (long)getpid(), (unsigned long)geteuid(), (unsigned long)getegid(), cwd,
        (unsigned long long)lim.rlim_cur, (unsigned long long)lim.rlim_max,
        argc, count, image_private);
    return 0;
}
