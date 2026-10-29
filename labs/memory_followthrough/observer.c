#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef MW_FIXED
#define MW_FIXED 0
#endif
extern void mw_dependency(unsigned char *, size_t);
extern void mw_missing_field(unsigned *, unsigned);
static volatile unsigned sink;

struct mw_node
{
    struct mw_node *next;
    unsigned char payload[24];
};

static void *root;
static void *interior;

static void escape(void *p)
{
    sink = (unsigned)((unsigned char *)p != NULL);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    unsigned char *p = malloc(8);
    if (!p)
        return 3;
    memset(p, 1, 8);
    printf("SETUP:allocation8:mode=%s:fixed=%d\nENTRY\n", argv[1], MW_FIXED);
    fflush(stdout);
    if (!strcmp(argv[1], "extent"))
    {
        size_t i = MW_FIXED ? 7 : 8;
        mw_dependency(p, i);
        sink = p[7];
    }
    else if (!strcmp(argv[1], "lower"))
    {
        ptrdiff_t i = MW_FIXED ? 0 : -1;
        p[i] = 7;
        sink = p[0];
    }
    else if (!strcmp(argv[1], "nul"))
    {
        if (MW_FIXED)
            p[7] = 0;
        sink = (unsigned)strlen((char *)p);
    }
    else if (!strcmp(argv[1], "pool"))
    {
        size_t logical = 4, index = 6;
        if (MW_FIXED && index >= logical)
            puts("DENIED:logical-extent");
        else
        {
            p[index] = 9;
            puts("SEMANTIC:logical-extent-violated:ASan-addressable");
        }
    }
    else if (!strcmp(argv[1], "canary"))
    {
        p[4] = 0xaa;
        if (MW_FIXED)
            puts("DENIED:before-canary");
        else
        {
            p[4] = 7;
            puts("CANARY:damage-detected-after-extent-violation");
        }
        sink = p[4];
    }
    else if (!strcmp(argv[1], "init"))
    {
        struct fb_record
        {
            unsigned length, mode;
        } *r = malloc(sizeof *r);

        if (!r)
            return 3;
        r->length = 3;
        mw_missing_field(&r->mode, MW_FIXED);
        struct fb_record copy = *r;
        if (copy.mode)
            sink = copy.length;
        free(r);
    }
    else if (!strcmp(argv[1], "lifetime"))
    {
        if (!MW_FIXED)
        {
            free(p);
            sink = *((volatile unsigned char *)p);
            p = NULL;
        }
        else
            sink = p[0];
    }
    else if (!strcmp(argv[1], "signed"))
    {
        volatile int n = INT_MAX;
        int result = MW_FIXED ? INT_MAX : n * 2;
        sink = (unsigned)result;
    }
    else if (!strcmp(argv[1], "free"))
    {
        if (MW_FIXED)
        {
            free(p);
            p = NULL;
        }
        else
        {
            free(p + 1);
            p = NULL;
        }
    }
    else if (!strcmp(argv[1], "leak"))
    {
        void *lost = malloc(32);
        struct mw_node *a = calloc(1, sizeof *a), *b = calloc(1, sizeof *b);
        root = malloc(24);
        unsigned char *q = malloc(8);
        if (!lost || !a || !b || !root || !q)
            return 3;
        a->next = b;
        b->next = a;
        interior = q + 1;
        escape(lost);
        escape(a);
        if (MW_FIXED)
        {
            free(lost);
            free(a);
            free(b);
            free(root);
            root = NULL;
            free(q);
            interior = NULL;
        }
        else
            puts("SEMANTIC:reachable-cache24-budget16:excess8");
    }
    else
        return 2;
    free(p);
    puts("EXIT:bounded-trial");
    return 0;
}
