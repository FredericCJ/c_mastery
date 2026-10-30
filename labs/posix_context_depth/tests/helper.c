#define _GNU_SOURCE
#include "test.h"
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

/* The "original helper" image that pc_launch/pc_spawn execute. It checks the
 * exact inputs it was given and reports what it inherited on one line, which
 * the launcher tests compare against the parent's own values. */

extern char **environ;

enum
{
    EXPECTED_ARGC = 3,
    EXPECTED_ENV_COUNT = 2,
    MISSING_CONFIG_STATUS = 23,
    CWD_BYTES = 4096,
    PRIVATE_INITIAL_VALUE = 7
};

/* Initialized data of this image, reported as the new image's private
 * initialization (CONTRACTS.md). */
static int image_private = PRIVATE_INITIAL_VALUE;

static size_t environment_count(void)
{
    size_t count = 0;
    while (environ[count])
    {
        ++count;
    }
    return count;
}

int main(int argc, char **argv)
{
    CHECK(argc == EXPECTED_ARGC && strcmp(argv[1], "") == 0 &&
          strcmp(argv[2], "with spaces") == 0);
    if (!getenv("PC_CONFIG"))
    {
        puts("PC_MISSING_CONFIG");
        return MISSING_CONFIG_STATUS;
    }
    CHECK(!strcmp(getenv("PC_CONFIG"), "synthetic=alpha"));
    CHECK(getenv("PC_SECOND") && !strcmp(getenv("PC_SECOND"), "beta"));
    /* Exactly the two supplied variables: nothing inherited, no PATH. */
    size_t count = environment_count();
    CHECK(count == EXPECTED_ENV_COUNT && !getenv("PATH"));
    char cwd[CWD_BYTES];
    struct rlimit limit;
    CHECK(getcwd(cwd, sizeof cwd) && getrlimit(RLIMIT_NOFILE, &limit) == 0);
    CHECK(image_private == PRIVATE_INITIAL_VALUE);
    printf(
        "PC_HELPER pid=%ld euid=%lu egid=%lu cwd=%s soft=%llu hard=%llu argc=%d env=%zu private=%d\n",
        (long)getpid(), (unsigned long)geteuid(), (unsigned long)getegid(), cwd,
        (unsigned long long)limit.rlim_cur, (unsigned long long)limit.rlim_max,
        argc, count, image_private);
    return 0;
}
