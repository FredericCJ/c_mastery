#define _GNU_SOURCE
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "EFFECTS_STACK_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(1);                                                       \
        }                                                                  \
    } while (0)
static uint32_t result;

static __attribute__((noinline)) uint32_t nested(unsigned depth)
{
    volatile unsigned char a[256];
    for (unsigned i = 0; i < 256; ++i)
    {
        a[i] = (unsigned char)(depth + i);
    }
    return a[0] + (depth ? nested(depth - 1) : 0);
}

static void *worker(void *p)
{
    (void)p;
    result = nested(3);
    return NULL;
}

int main(void)
{
    long page = sysconf(_SC_PAGESIZE);
    CHECK(page > 0);
    size_t n = 1024 * 1024, total = n + 2 * (size_t)page;
    unsigned char *map =
        mmap(NULL, total, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(map != MAP_FAILED);
    unsigned char *stack = map + page;
    CHECK(mprotect(stack, n, PROT_READ | PROT_WRITE) == 0);
    memset(stack, 0xa5, n);
    pthread_attr_t attr;
    CHECK(pthread_attr_init(&attr) == 0 &&
          pthread_attr_setstack(&attr, stack, n) == 0);
    pthread_t thread;
    CHECK(pthread_create(&thread, &attr, worker, NULL) == 0 &&
          pthread_join(thread, NULL) == 0 && result == 6);
    size_t first = 0;
    while (first < n && stack[first] == 0xa5)
    {
        ++first;
    }
    CHECK(first < n);
    printf(
        "{\"stack_bytes\":%zu,\"changed_suffix_bytes\":%zu,\"pattern\":165,\"includes_pthread_startup\":true,\"architecture\":\"x86_64-downward\"}\n",
        n, n - first);
    CHECK(pthread_attr_destroy(&attr) == 0 && munmap(map, total) == 0);
    return 0;
}
