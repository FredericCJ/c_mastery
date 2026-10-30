/*
 * Stack high-water experiment (Linux/x86-64, GNU extensions).
 *
 * Maps a private region with an inaccessible guard page on each side, fills the
 * usable middle with a known byte pattern, runs a recursive worker on it as a
 * thread stack, then reports the span from the lowest changed byte up to the
 * top of the region. The worker runs in its own thread, which is joined before
 * the region is read. The stack grows downward on x86-64, so a span measured
 * from the top approximates the stack used, including thread start-up.
 */
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

#define STACK_BYTES (1024u * 1024u)
#define FILL_PATTERN 0xa5
#define FRAME_ARRAY_BYTES 256u
#define RECURSION_DEPTH 3u

/* nested(3) returns 3 + 2 + 1 + 0: each activation adds its own depth. */
#define EXPECTED_RESULT 6u

static uint32_t result;

/* Each activation keeps FRAME_ARRAY_BYTES live in a volatile array. */
static __attribute__((noinline)) uint32_t nested(unsigned depth)
{
    volatile unsigned char frame[FRAME_ARRAY_BYTES];
    for (unsigned i = 0; i < FRAME_ARRAY_BYTES; ++i)
    {
        frame[i] = (unsigned char)(depth + i);
    }
    return frame[0] + (depth ? nested(depth - 1) : 0);
}

static void *worker(void *unused)
{
    (void)unused;
    result = nested(RECURSION_DEPTH);
    return NULL;
}

/* Bytes counted from the low end that still hold FILL_PATTERN. */
static size_t untouched_prefix(const unsigned char *stack, size_t bytes)
{
    size_t first = 0;
    while (first < bytes && stack[first] == FILL_PATTERN)
    {
        ++first;
    }
    return first;
}

int main(void)
{
    long page = sysconf(_SC_PAGESIZE);
    CHECK(page > 0);
    size_t stack_bytes = STACK_BYTES;
    size_t total_bytes = stack_bytes + 2 * (size_t)page;
    unsigned char *map =
        mmap(NULL, total_bytes, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(map != MAP_FAILED);
    unsigned char *stack = map + page;
    CHECK(mprotect(stack, stack_bytes, PROT_READ | PROT_WRITE) == 0);
    memset(stack, FILL_PATTERN, stack_bytes);

    pthread_attr_t attr;
    CHECK(pthread_attr_init(&attr) == 0 &&
          pthread_attr_setstack(&attr, stack, stack_bytes) == 0);
    pthread_t thread;
    CHECK(pthread_create(&thread, &attr, worker, NULL) == 0 &&
          pthread_join(thread, NULL) == 0 && result == EXPECTED_RESULT);

    size_t first_changed = untouched_prefix(stack, stack_bytes);
    CHECK(first_changed < stack_bytes);
    printf(
        "{\"stack_bytes\":%zu,\"changed_suffix_bytes\":%zu,\"pattern\":%d,\"includes_pthread_startup\":true,\"architecture\":\"x86_64-downward\"}\n",
        stack_bytes, stack_bytes - first_changed, FILL_PATTERN);
    CHECK(pthread_attr_destroy(&attr) == 0 && munmap(map, total_bytes) == 0);
    return 0;
}
