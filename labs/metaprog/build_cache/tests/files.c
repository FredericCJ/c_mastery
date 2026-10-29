#define _POSIX_C_SOURCE 200809L
#include "cache_file.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "MPO_CHECK MPB_FILE_CHECK line%d %s\n", __LINE__, \
                    #x);                                                      \
            return 1;                                                         \
        }                                                                     \
    } while (0)

struct mp_cuts
{
    unsigned calls, stop, rename_at;
    int exit_process;
};

static int owned_fd = -1;
static char owned_directory[80];

static void cleanup(void)
{
    if (owned_fd >= 0)
    {
        (void)unlinkat(owned_fd, "node0.cache", 0);
        (void)unlinkat(owned_fd, "node0.stage", 0);
        (void)close(owned_fd);
        owned_fd = -1;
    }
    if (owned_directory[0])
    {
        (void)rmdir(owned_directory);
    }
}

static int boundary(void *p, unsigned stage, size_t transferred)
{
    struct mp_cuts *c = p;
    (void)transferred;
    ++c->calls;
    if (stage == MPB_RENAME)
    {
        c->rename_at = c->calls;
    }
    if (c->calls == c->stop)
    {
        if (c->exit_process)
        {
            _exit(70);
        }
        return 1;
    }
    return 0;
}

int main(void)
{
    char directory[] = "/tmp/cmastery-build-cache-XXXXXX";
    unsigned char old[80], next[80], bad[80];
    int fd, status;
    struct mp_cuts observation = {0};
    for (unsigned i = 0; i < 80; ++i)
    {
        old[i] = next[i] = (unsigned char)i;
    }
    next[24] ^= 1;
    memcpy(bad, next, 80);
    bad[79] ^= 1;
    CHECK(mkdtemp(directory) != NULL);
    strcpy(owned_directory, directory);
    CHECK(atexit(cleanup) == 0);
    fd = open(directory, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(fd >= 0);
    owned_fd = fd;
    CHECK(mpb_file_match(fd, 0, old, 80) == MPB_MISS);
    CHECK(mpb_file_put(fd, 0, next, 80, boundary, &observation) == MPB_OK &&
          observation.calls == 17 && observation.rename_at == 16);
    CHECK(mpb_file_match(fd, 0, next, 80) == MPB_OK &&
          mpb_file_match(fd, 0, bad, 80) == MPB_MISS);
    for (unsigned cut = 1; cut <= observation.calls; ++cut)
    {
        CHECK(mpb_file_recover(fd, 0) == MPB_OK &&
              mpb_file_put(fd, 0, old, 80, NULL, NULL) == MPB_OK);
        pid_t child = fork();
        CHECK(child >= 0);
        if (!child)
        {
            struct mp_cuts c = {0, cut, 0, 1};
            int r = mpb_file_put(fd, 0, next, 80, boundary, &c);
            _exit(r == MPB_OK ? 0 : 71);
        }
        CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
              WEXITSTATUS(status) == 70);
        CHECK(mpb_file_match(fd, 0, cut < observation.rename_at ? old : next,
                             80) == MPB_OK);
    }
    for (unsigned cut = 1; cut <= observation.calls; ++cut)
    {
        CHECK(mpb_file_recover(fd, 0) == MPB_OK &&
              mpb_file_put(fd, 0, old, 80, NULL, NULL) == MPB_OK);
        struct mp_cuts c = {0, cut, 0, 0};
        int r = mpb_file_put(fd, 0, next, 80, boundary, &c);
        CHECK(r == (cut < observation.rename_at ? MPB_IO : MPB_UNCERTAIN));
        CHECK(mpb_file_match(fd, 0, cut < observation.rename_at ? old : next,
                             80) == MPB_OK);
    }
    struct mpb_node node = {0};
    struct mpb_plan plan;
    unsigned char encoded[800];
    size_t length = 0;
    node.revision = -7;
    node.inputs[MPB_SCHEMA].length = 1;
    node.inputs[MPB_SCHEMA].bytes[0] = 2;
    CHECK(mpb_plan(&node, 1, 1, &plan) == MPB_OK &&
          mpb_encode(&plan.entries[0], 0, encoded, sizeof encoded, &length) ==
              MPB_OK);
    CHECK(mpb_file_put(fd, 0, encoded, length, NULL, NULL) == MPB_OK &&
          mpb_file_match(fd, 0, encoded, length) == MPB_OK);
    CHECK(mpb_file_recover(fd, 0) == MPB_OK &&
          unlinkat(fd, "node0.cache", 0) == 0 && close(fd) == 0);
    owned_fd = -1;
    CHECK(rmdir(directory) == 0);
    owned_directory[0] = 0;
    puts(
        "MPB_FILES process-cuts17 refusal-boundaries17 old-or-new prefix-corruption-refused actual-codec-entry no-power-cut-claim");
    return 0;
}
