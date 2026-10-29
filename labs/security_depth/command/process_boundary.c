#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x)                                           \
    do                                                     \
    {                                                      \
        if (!(x))                                          \
        {                                                  \
            fprintf(stderr, "SC_PROCESS_ASSERT:%s\n", #x); \
            exit(41);                                      \
        }                                                  \
    } while (0)
static uint32_t private_counter = 7;

static void logical_module(uint32_t *address)
{
    *address = 99;
}

int main(void)
{
    int f[2], status;
    pid_t child;
    uint32_t observed = 0;
    logical_module(&private_counter);
    CHECK(private_counter == 99);
    private_counter = 7;
    CHECK(pipe(f) == 0);
    child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        close(f[0]);
        logical_module(&private_counter);
        if (write(f[1], &private_counter, sizeof private_counter) !=
            (ssize_t)sizeof private_counter)
        {
            _exit(1);
        }
        close(f[1]);
        _exit(0);
    }
    CHECK(close(f[1]) == 0);
    CHECK(read(f[0], &observed, sizeof observed) == (ssize_t)sizeof observed);
    CHECK(close(f[0]) == 0);
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    CHECK(observed == 99 && private_counter == 7);
    puts("OBS process same_address_space_write=observed "
         "child_private_write=isolated no_complete_sandbox_claim=1");
    return 0;
}
