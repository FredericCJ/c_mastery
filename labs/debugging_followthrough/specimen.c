#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef DX_FIXED
#define DX_FIXED 0
#endif
#ifndef DX_PAREN
#define DX_PAREN 0
#endif
#ifndef DX_LOGGING
#define DX_LOGGING 1
#endif
#ifndef DX_CLAMP
#define DX_CLAMP 1
#endif
#if DX_LOGGING
#define DIAG(x) ((void)(x))
#else
#define DIAG(x) ((void)0)
#endif
volatile unsigned used, capacity = 4, violations, observed, diagnostic_counter,
                        validated, cleaned;

__attribute__((noinline)) void dx_marker(unsigned stage)
{
    __asm__ volatile("" : : "r"(stage) : "memory");
}

__attribute__((noinline)) void dx_callback(void)
{
    observed = used;
    dx_marker(30);
}

__attribute__((noinline)) int dx_acquire(void)
{
    dx_marker(10);
#if DX_FIXED
    if (used >= capacity)
    {
        dx_marker(12);
        return 0;
    }
    used++;
#else
    used++; /* First invalid committed state on the fifth acquisition. */
    dx_callback();
    if (used > capacity)
    {
        violations++;
        if (DX_CLAMP)
            used = capacity;
        return 0;
    }
#endif
    dx_marker(11);
    return 1;
}

__attribute__((noinline)) void dx_private_candidate(void)
{
    unsigned candidate = used + 1;
    dx_marker(40);
    if (candidate > capacity)
        violations++;
    used = candidate;
    dx_marker(41);
}

__attribute__((noinline)) int dx_scope(int input)
{
    int status = 7;
    dx_marker(20);
    {
        int status = 11;
        dx_marker((unsigned)status);
    }
    return status + input;
}

static inline int dx_inline_scope(int input)
{
    int status = 13;
    dx_marker((unsigned)status);
    return input + status;
}

__attribute__((noinline)) int dx_tail_leaf(int x)
{
    int vanished = rand();
    dx_marker(22);
    (void)vanished;
    return x + 1;
}

__attribute__((noinline)) int dx_tail(int x)
{
    return dx_tail_leaf(x);
}

static inline void log_inline(unsigned n)
{
    (void)n;
}

static int assignment(unsigned n)
{
#if DX_FIXED
    if (n == 0)
        return 0;
#elif DX_PAREN
    if ((n = 0))
        return 0;
#else
    if (n = 0)
        return 0;
#endif
    if (n > capacity)
        return 0;
    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 2;
    const char *configured = getenv("DX_CAP");
    if (configured)
        capacity = (unsigned)strtoul(configured, NULL, 10);
    if (!strcmp(argv[1], "assignment"))
    {
        unsigned inputs[] = {0, 1, 4, 5};
        for (unsigned i = 0; i < 4; i++)
            printf("ADMIT:%u:%d\n", inputs[i], assignment(inputs[i]));
        return 0;
    }
    if (!strcmp(argv[1], "macros"))
    {
        validated++;
        DIAG(diagnostic_counter++);
        cleaned++;
        unsigned macro = diagnostic_counter;
        log_inline(diagnostic_counter++);
        printf("MACROS:%u:%u:%u:%u\n", validated, cleaned, macro,
               diagnostic_counter);
        return 0;
    }
    if (!strcmp(argv[1], "scope"))
    {
        int status = 2;
        int result = dx_scope(status);
        result += dx_inline_scope(status);
        dx_marker(21);
        printf("SCOPES:%d:%d\n", status, result);
        return dx_tail(0) != 1;
    }
    if (!strcmp(argv[1], "candidate"))
    {
        used = 4;
        dx_private_candidate();
        return 0;
    }
    if (!strcmp(argv[1], "counter"))
    {
        for (unsigned i = 0; i < 5; i++)
            dx_acquire();
        used--;
        printf("COUNTER:%u:%u:%u\n", used, violations, observed);
        return 0;
    }
    if (!strcmp(argv[1], "history"))
    {
        printf("HISTORY:");
        for (unsigned i = 0; i < 5; i++)
        {
            dx_acquire();
            printf("%u,", used);
        }
        used--;
        printf("%u\n", used);
        return 0;
    }
    if (!strcmp(argv[1], "relocate"))
    {
        unsigned *p = malloc(sizeof *p);
        if (!p)
            return 3;
        *p = 4;
        dx_marker(50);
        unsigned *q = malloc(sizeof *q);
        if (!q)
        {
            free(p);
            return 3;
        }
        *q = *p;
        dx_marker(51);
        free(p);
        p = q;
        dx_marker(52);
        *p = 5;
        dx_marker(53);
        free(p);
        return 0;
    }
    if (!strcmp(argv[1], "parser"))
    {
        if (argc != 3 || strlen(argv[2]) > 18)
            return 2;
        const char *s = argv[2];
        size_t n = strlen(s);
        if (n < 2 || s[0] != '[' || s[n - 1] != ']')
            return 2;
        for (size_t i = 1; i + 1 < n; i++)
            if (s[i] != 'A' && s[i] != 'N' && s[i] != 'T' && s[i] != 'C')
                return 2;
        if (strchr(s, 'T'))
        {
            struct timespec t = {2, 0};
            nanosleep(&t, NULL);
            return 3;
        }
        if (strchr(s, 'C'))
        {
            raise(SIGABRT);
            return 3;
        }
        const char *mode = getenv("DX_MODE");
        capacity = mode && !strcmp(mode, "small") ? 3 : 4;
        used = capacity;
        for (size_t i = 1; i + 1 < n; i++)
            if (s[i] == 'A')
                dx_acquire();
        printf("TRANSITION:admission:seed=%s:used=%u:violations=%u\n",
               getenv("DX_SEED") ? getenv("DX_SEED") : "0", used, violations);
        return violations ? 1 : 0;
    }
    return 2;
}
