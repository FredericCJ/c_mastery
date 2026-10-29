#include "allocation.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "HOST-ALLOCATION-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

struct over_aligned
{
    _Alignas(64) unsigned char payload[64];
};

static int supported(void *c, size_t a)
{
    (void)c;
    return a == 1 || a == _Alignof(max_align_t) || a == 64;
}

static void *allocate(void *c, size_t n, size_t a)
{
    (void)c;
    return a > _Alignof(max_align_t) ? aligned_alloc(a, n) : malloc(n);
}

static void *resize(void *c, void *p, size_t n)
{
    (void)c;
    return realloc(p, n);
}

static void release(void *c, void *p)
{
    (void)c;
    free(p);
}

static int copy(void *c, void *d, const void *s, size_t n)
{
    (void)c;
    memcpy(d, s, n);
    return 0;
}

int main(void)
{
    struct la_ops ops = {NULL,
                         supported,
                         allocate,
                         resize,
                         release,
                         copy,
                         _Alignof(max_align_t)};
    struct la_owner owner = {0};
    CHECK(la_create(ops, &owner, 4, 1, 1) == LA_OK);
    memcpy(owner.data, "ABCD", 4);
    CHECK(la_resize_native(ops, &owner, 8, 1) == LA_OK && owner.length == 8 &&
          !memcmp(owner.data, "ABCD", 4));
    CHECK(la_release(ops, &owner) == LA_OK);
    CHECK(_Alignof(struct over_aligned) == 64 &&
          sizeof(struct over_aligned) == 64);
    CHECK(la_create(ops, &owner, 2, sizeof(struct over_aligned),
                    _Alignof(struct over_aligned)) == LA_OK);
    struct over_aligned *p = owner.data;
    for (unsigned i = 0; i < 2; ++i)
        for (unsigned j = 0; j < 64; ++j)
            p[i].payload[j] = (unsigned char)(i * 64 + j);
    CHECK((uintptr_t)p % 64 == 0 && owner.capacity == 128);
    CHECK(la_resize_copy(ops, &owner, 3, sizeof(struct over_aligned)) == LA_OK);
    p = owner.data;
    for (unsigned i = 0; i < 2; ++i)
        for (unsigned j = 0; j < 64; ++j)
            CHECK(p[i].payload[j] == i * 64 + j);
    CHECK((uintptr_t)p % 64 == 0 && owner.capacity == 192);
    CHECK(la_release(ops, &owner) == LA_OK);
    unsigned char *zero = calloc(7, 3);
    CHECK(zero);
    for (unsigned i = 0; i < 21; ++i)
        CHECK(zero[i] == 0);
    free(zero);

    struct values
    {
        void *pointer;
        double number;
    };
    struct values *values = malloc(sizeof *values);
    CHECK(values);
    *values = (struct values){.pointer = NULL, .number = 0.0};
    CHECK(values->pointer == NULL && values->number == 0.0);
    free(values); /* Semantic initialization, no inferred all-zero object
                     representation. */
    printf("HOST-ALLOCATION-PASS:fundamental=%zu:extended=64:malloc:calloc:"
           "realloc:aligned_alloc\n",
           _Alignof(max_align_t));
    return 0;
}
