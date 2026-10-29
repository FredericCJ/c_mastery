#include "test.h"
#ifndef AR_STATIC_POLICY
#define AR_STATIC_POLICY 1
#endif
struct ac_sink
{
    unsigned char bytes[8];
    size_t length;
    unsigned fail;
};
typedef struct ar_result (*ac_write_fn)(void *, const unsigned char *, size_t);

static struct ar_result write_width(void *p, const unsigned char *b, size_t n,
                                    size_t width)
{
    struct ac_sink *s = p;
    if (s->fail)
        return (struct ar_result){0, AR_FAILED};
    if (n > width)
        n = width;
    for (size_t i = 0; i < n; ++i)
        s->bytes[s->length++] = b[i];
    return (struct ar_result){(ptrdiff_t)n, AR_OK};
}

static struct ar_result one(void *p, const unsigned char *b, size_t n)
{
    return write_width(p, b, n, 1);
}

static struct ar_result two(void *p, const unsigned char *b, size_t n)
{
    return write_width(p, b, n, 2);
}

__attribute__((noinline)) static struct ar_result
direct(void *p, const unsigned char *b, size_t n)
{
#if AR_STATIC_POLICY == 1
    return one(p, b, n);
#else
    return two(p, b, n);
#endif
}

__attribute__((noinline)) static int pump(ac_write_fn w, struct ac_sink *s)
{
    const unsigned char bytes[] = {1, 2, 3};
    size_t offset = 0;
    while (offset < 3)
    {
        struct ar_result r = w(s, bytes + offset, 3 - offset);
        if (r.state != AR_OK)
            return AR_FAILED;
        if (r.count <= 0 || (size_t)r.count > 3 - offset)
            return AR_CONTRACT;
        offset += (size_t)r.count;
    }
    return AR_OK;
}

int main(void)
{
    ac_write_fn policies[] = {one, two, direct};
    for (unsigned i = 0; i < 3; ++i)
    {
        struct ac_sink s = {0};
        CHECK(pump(policies[i], &s) == AR_OK && s.length == 3 &&
              !memcmp(s.bytes, (unsigned char[]){1, 2, 3}, 3));
        s = (struct ac_sink){.fail = 1};
        CHECK(pump(policies[i], &s) == AR_FAILED && s.length == 0);
    }
    printf("PASS compiletime=%d runtime2policies sizeofFunctionPointer=%zu "
           "sizeofSink=%zu same progress/error contract\n",
           AR_STATIC_POLICY, sizeof(ac_write_fn), sizeof(struct ac_sink));
}
