#include "fileio.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "FILEIO-CHECK: %s line%d\n", #x, __LINE__); \
            return 1;                                                   \
        }                                                               \
    } while (0)

struct source
{
    unsigned step, reads;
    int error;
};

static struct li_read_result read_step(void *v, unsigned char *p, size_t cap)
{
    struct source *s = v;
    ++s->reads;
    if (cap != 2)
        return (struct li_read_result){0, LI_ERROR};
    if (s->step++ == 0)
    {
        p[0] = 'a';
        p[1] = 'b';
        return (struct li_read_result){2, LI_MORE};
    }
    if (s->error)
        return (struct li_read_result){0, LI_ERROR};
    if (s->step == 2)
    {
        p[0] = 'c';
        return (struct li_read_result){1, LI_MORE};
    }
    return (struct li_read_result){0, LI_EOF};
}

struct sink
{
    unsigned char bytes[32];
    size_t count, quota, fail_after;
    unsigned calls;
    int wait;
};

static ptrdiff_t write_step(void *v, const uint8_t *p, size_t n)
{
    struct sink *s = v;
    ++s->calls;
    if (s->wait)
    {
        s->wait = 0;
        return -2;
    }
    if (s->count >= s->fail_after)
        return -1;
    size_t k = n < s->quota ? n : s->quota;
    if (k > s->fail_after - s->count)
        k = s->fail_after - s->count;
    if (k > sizeof s->bytes - s->count)
        return -1;
    memcpy(s->bytes + s->count, p, k);
    s->count += k;
    return (ptrdiff_t)k;
}

struct release
{
    unsigned closes, flushes;
    int fail;
};

static int flush_step(void *v)
{
    struct release *r = v;
    ++r->flushes;
    return r->fail;
}

static int close_step(void *v)
{
    struct release *r = v;
    ++r->closes;
    return r->fail;
}

static struct li_read_result result_step(void *v, unsigned char *p, size_t cap)
{
    if (cap >= 2)
    {
        p[0] = 'a';
        p[1] = 'b';
    }
    return *(struct li_read_result *)v;
}

int main(void)
{
    for (unsigned error = 0; error < 2; ++error)
        for (size_t quota = 1; quota <= 2; ++quota)
            for (unsigned budget = 1; budget <= 4; ++budget)
                for (int wait = 0; wait < 2; ++wait)
                {
                    struct source src = {0, 0, (int)error};
                    struct sink dst = {{0}, 0, quota, 32, 0, wait};
                    struct li_copy c;
                    li_init(&c);
                    enum li_status st = LI_PENDING;
                    unsigned retries = 0;
                    while (st == LI_PENDING && retries++ < 32)
                    {
                        unsigned before = src.reads + dst.calls;
                        st = li_pump(&c, read_step, &src, write_step, &dst,
                                     budget);
                        CHECK(src.reads + dst.calls - before <= budget);
                        CHECK(c.offset <= c.length && c.length <= 2 &&
                              c.written_total == dst.count);
                    }
                    CHECK(st == (error ? LI_READ_ERROR : LI_DONE) &&
                          dst.count == (error ? 2u : 3u) &&
                          !memcmp(dst.bytes, error ? "ab" : "abc", dst.count));
                    unsigned before = src.reads + dst.calls;
                    CHECK(li_pump(&c, read_step, &src, write_step, &dst, 4) ==
                              st &&
                          before == src.reads + dst.calls);
                }
    for (size_t cutoff = 0; cutoff < 3; ++cutoff)
    {
        struct source src = {0};
        struct sink dst = {{0}, 0, 1, cutoff, 0, 0};
        struct li_copy c;
        li_init(&c);
        enum li_status st = LI_PENDING;
        for (unsigned i = 0; i < 32 && st == LI_PENDING; ++i)
            st = li_pump(&c, read_step, &src, write_step, &dst, 1);
        CHECK(st == LI_WRITE_ERROR && dst.count == cutoff &&
              !memcmp(dst.bytes, "abc", cutoff) && c.written_total == cutoff);
    }
    for (int owned = 0; owned < 2; ++owned)
        for (int output = 0; output < 2; ++output)
            for (int fail = 0; fail < 2; ++fail)
            {
                struct release r = {0, 0, fail};
                struct li_owner o = {&r, owned, output};
                enum li_status expected = fail ? (owned    ? LI_CLOSE_ERROR
                                                  : output ? LI_FLUSH_ERROR
                                                           : LI_DONE)
                                               : LI_DONE;
                CHECK(li_release(&o, flush_step, close_step) == expected);
                CHECK(!o.handle && r.closes == (unsigned)owned &&
                      r.flushes == (unsigned)(!owned && output));
                CHECK(li_release(&o, flush_step, close_step) == LI_CLOSED);
                CHECK(r.closes == (unsigned)owned);
            }
    struct li_read_result results[] = {
        {0, LI_MORE},  {1, LI_WAIT},
        {3, LI_MORE},  {0, (enum li_read_status) - 1},
        {2, LI_ERROR}, {2, LI_EOF}};
    for (size_t i = 0; i < sizeof results / sizeof *results; ++i)
    {
        struct li_copy c;
        li_init(&c);
        struct sink dst = {{0}, 0, 1, 32, 0, 0};
        enum li_status st = LI_PENDING;
        for (unsigned j = 0; j < 10 && st == LI_PENDING; ++j)
            st = li_pump(&c, result_step, &results[i], write_step, &dst, 1);
        CHECK(st == (i < 4 ? LI_PROTOCOL : i == 4 ? LI_READ_ERROR : LI_DONE));
        CHECK(dst.count == (i < 4 ? 0u : 2u));
    }
    puts(
        "FILEIO-PASS:32 resumable traces,three terminal prefixes,eight release "
        "policies,six result boundaries");
    return 0;
}
