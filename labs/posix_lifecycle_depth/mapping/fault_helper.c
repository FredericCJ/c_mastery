/* Original deliberately faulting specimens; isolated subprocess, no core dump.
   This executable is intentionally not instrumented by a signal-intercepting
   sanitizer: its externally observed Linux termination signal is the subject. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#ifndef PM_FAULT_MUTANT
#define PM_FAULT_MUTANT 0
#endif
int main(int argc, char **argv)
{
    if (argc != 5)
    {
        return 2;
    }
    struct rlimit limit = {0, 0};
    if (setrlimit(RLIMIT_CORE, &limit) < 0)
    {
        return 3;
    }
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0)
    {
        return 4;
    }
    int fd = open(argv[2], O_RDWR | O_CLOEXEC);
    if (fd < 0)
    {
        return 5;
    }
    unsigned char *p = mmap(NULL, (size_t)(2 * page), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
    {
        return 7;
    }
    p[page] = 42;
    int ready = atoi(argv[3]), release = atoi(argv[4]);
    char byte;
    if (write(ready, "R", 1) != 1 || read(release, &byte, 1) != 1)
    {
        return 6;
    }
    close(ready);
    close(release);
    if (argv[1][0] == '1' && PM_FAULT_MUTANT != 1)
    {
        if (mprotect(p, (size_t)page, PROT_NONE) < 0)
        {
            return 8;
        }
        volatile unsigned char value = p[0];
        (void)value;
    }
    if (argv[1][0] == '2')
    {
        volatile unsigned char value = p[PM_FAULT_MUTANT == 2 ? 0 : page];
        (void)value;
    }
    if (munmap(p, (size_t)(2 * page)) < 0 || close(fd) < 0)
    {
        return 10;
    }
    return 0;
}
