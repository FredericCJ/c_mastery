#define _GNU_SOURCE
#include "posix_lifecycle.h"
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "PL_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

/* Counts open descriptors by listing /proc/self/fd (Linux). The count
   includes the descriptor opendir() itself holds, equally before and after,
   so only differences between two calls are meaningful. Returns -1 if the
   listing cannot be opened. */
static int descriptors(void)
{
    DIR *directory = opendir("/proc/self/fd");
    if (!directory)
    {
        return -1;
    }
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(directory)))
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            ++count;
        }
    }
    closedir(directory);
    return count;
}

enum
{
    PATH_BUFFER_BYTES = 4096,
    SENTINEL = 999 /* value no real launch result can produce */
};

/* pl_launch(): the three result kinds, injected acquisition failures, and the
   guarantee that no descriptor leaks and no child is left to reap. */
static int test_launch(const char *helper)
{
    int initial = descriptors();
    CHECK(initial >= 0);
    char *args[] = {(char *)helper, "zero", NULL};
    struct pl_launch result = {PL_SIGNALED, SENTINEL};
    CHECK(pl_launch(helper, args, 0, &result) == 0);
    CHECK(result.kind == PL_EXITED && result.value == 0);

    /* A program that itself exits 127 must not be mistaken for an exec
       failure: only the error channel says that. */
    args[1] = "127";
    CHECK(pl_launch(helper, args, 0, &result) == 0);
    CHECK(result.kind == PL_EXITED && result.value == 127);

    char absent[PATH_BUFFER_BYTES];
    int length = snprintf(absent, sizeof absent, "%s.missing", helper);
    CHECK(length > 0 && (size_t)length < sizeof absent);
    struct stat metadata;
    errno = 0;
    CHECK(lstat(absent, &metadata) == -1 && errno == ENOENT);
    CHECK(pl_launch(absent, args, 0, &result) == 0);
    CHECK(result.kind == PL_EXEC_ERROR && result.value == ENOENT);

    args[1] = "signal";
    CHECK(pl_launch(helper, args, 0, &result) == 0);
    CHECK(result.kind == PL_SIGNALED && result.value == SIGTERM);

    /* fail_stage 1 and 2 inject a pipe and a fork failure. */
    for (unsigned fail = 1; fail <= 2; ++fail)
    {
        result = (struct pl_launch){PL_SIGNALED, SENTINEL};
        CHECK(pl_launch(helper, args, fail, &result) ==
              (fail == 1 ? EMFILE : EAGAIN));
        CHECK(result.kind == PL_SIGNALED && result.value == SENTINEL);
        CHECK(descriptors() == initial);
    }
    CHECK(pl_launch(NULL, args, 0, &result) == EINVAL);
    CHECK(descriptors() == initial);
    errno = 0;
    CHECK(waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD);
    return 0;
}

/* Sum of squares 1..count, the value pl_pool() must report for `count`
   successfully created workers. */
static unsigned expected_sum(unsigned count)
{
    unsigned sum = 0;
    for (unsigned i = 1; i <= count; ++i)
    {
        sum += i * i;
    }
    return sum;
}

/* pl_pool(): for every worker count 0..4 and every injected creation
   failure ordinal, exactly the successful prefix is created, joined and
   summed; invalid requests leave the output untouched. */
static int test_pool(void)
{
    for (unsigned workers = 0; workers <= 4; ++workers)
    {
        for (unsigned fail = 0; fail <= workers; ++fail)
        {
            struct pl_pool result = {99, 99, 99, 99, 99};
            unsigned made = fail ? fail - 1 : workers;
            CHECK(pl_pool(workers, fail, &result) == 0);
            CHECK(result.created == made && result.joined == made &&
                  result.completed == made);
            CHECK(result.sum == expected_sum(made) &&
                  result.create_error == (fail ? EAGAIN : 0));
        }
    }
    struct pl_pool sentinel = {99, 99, 99, 99, 99};
    struct pl_pool result = sentinel;
    CHECK(pl_pool(5, 0, &result) == EINVAL);
    CHECK(result.created == 99 && result.sum == 99 &&
          result.create_error == 99);
    CHECK(pl_pool(1, 2, &result) == EINVAL);
    CHECK(pl_pool(1, 0, NULL) == EINVAL);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        return 2;
    }
    int failed;
    if (!strcmp(argv[1], "launch"))
    {
        failed = test_launch(argv[2]);
    }
    else if (!strcmp(argv[1], "pool"))
    {
        failed = test_pool();
    }
    else
    {
        return 2;
    }
    if (failed)
    {
        return failed;
    }
    printf("PASS posix_lifecycle_depth.%s\n", argv[1]);
    return 0;
}
