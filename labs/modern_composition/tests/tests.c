#include "composition.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "MC_CHECK: line %d: %s\n", __LINE__, #x); \
            exit(1);                                                  \
        }                                                             \
    } while (0)

static void publish(void)
{
    struct ow_owner empty = {0};
    struct mc_publish invalid = mc_publish_pair(
        NULL, (size_t)INT32_MAX + 1, (struct v4_allocator){0}, &empty);
    CHECK(invalid.parse.tag == ER_INVALID && invalid.storage == OW_OK &&
          !invalid.committed);
    for (size_t failure = 0; failure <= 3; ++failure)
    {
        struct ow_allocator pool = {0};
        struct v4_allocator a = ow_allocator(&pool);
        struct ow_owner published = {0};
        size_t sizes[3] = {1, 1, 1};
        CHECK(ow_construct(&published, a, sizes) == OW_OK);
        for (unsigned i = 0; i < 3; ++i)
        {
            published.block[i][0] = (int32_t)(90 + i);
        }
        struct ow_owner old = published;
        pool.fail_at = failure ? pool.attempts + failure : 0;
        struct mc_publish bad =
            mc_publish_pair((const unsigned char *)"3:x", 3, a, &published);
        CHECK(!bad.committed && bad.parse.tag == ER_INVALID &&
              bad.storage == OW_OK);
        CHECK(pool.attempts == 3 && pool.live == 3);
        struct mc_publish result =
            mc_publish_pair((const unsigned char *)"3:7", 3, a, &published);
        CHECK(result.parse.tag == ER_OK);
        if (failure)
        {
            CHECK(!result.committed && result.storage == OW_NOMEM &&
                  pool.live == 3);
            for (unsigned i = 0; i < 3; ++i)
            {
                CHECK(published.block[i] == old.block[i] &&
                      published.block[i][0] == (int32_t)(90 + i));
            }
        }
        else
        {
            CHECK(result.committed && pool.live == 3 && pool.releases == 3);
            CHECK(published.block[0][0] == 10 && published.block[1][0] == 2 &&
                  published.block[2][0] == 3);
        }
        CHECK(ow_destroy(&published) == OW_OK && pool.live == 0 &&
              pool.invalid_releases == 0);
    }
}

struct mc_backend
{
    struct mc_read *read;
    unsigned mode, reads, arms, disarms, cleanups, notifications;
    bool borrowed, release_on_notify;
    enum msg_status status;
    unsigned progress;
};

static struct co_read_result read_bytes(void *context, unsigned char *out,
                                        size_t n)
{
    struct mc_backend *b = context;
    ++b->reads;
    if (b->reads == 1)
    {
        CHECK(n >= 1);
        out[0] = 'a';
        return (struct co_read_result){CO_READ_DATA, 1};
    }
    if (b->mode == 0)
    {
        return (struct co_read_result){CO_READ_AGAIN, 0};
    }
    if (b->mode == 2)
    {
        CHECK(n >= 1);
        out[0] = 'b';
        return (struct co_read_result){CO_READ_ERROR, 1};
    }
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = (unsigned char)('b' + i);
    }
    return (struct co_read_result){CO_READ_DATA, n};
}

static int arm(void *context, uint32_t generation)
{
    struct mc_backend *b = context;
    CHECK(generation == 7 && !b->borrowed);
    b->borrowed = true;
    ++b->arms;
    return 0;
}

static void disarm(void *context, uint32_t generation)
{
    struct mc_backend *b = context;
    CHECK(generation == 7 && b->borrowed);
    b->borrowed = false;
    ++b->disarms;
}

static void cleanup(void *context)
{
    struct mc_backend *b = context;
    CHECK(!b->borrowed && b->disarms == 1);
    ++b->cleanups;
}

static void receiver(void *context, enum msg_status status, unsigned value)
{
    struct mc_backend *b = context;
    CHECK(!b->borrowed && b->cleanups == 1 && b->notifications == 0);
    b->status = status;
    b->progress = value;
    ++b->notifications;
    if (b->release_on_notify)
    {
        free(b->read);
        b->read = NULL;
    }
}

static void async_read(void)
{
    for (unsigned mode = 0; mode < 3; ++mode)
    {
        for (unsigned release = 0; release < 2; ++release)
        {
            struct mc_backend b = {0};
            b.mode = mode;
            b.release_on_notify = release;
            struct mc_read *r = malloc(sizeof(*r));
            CHECK(r);
            b.read = r;
            unsigned char out[8];
            memset(out, 0x5a, sizeof(out));
            struct co_async_port port = {&b, read_bytes, arm, disarm};
            CHECK(mc_read_init(r, port, 3, 7, out, receiver, &b, cleanup, &b) ==
                  MSG_OK);
            enum msg_status status = mc_read_poll(r);
            if (mode == 0)
            {
                CHECK(status == MSG_BUSY && b.notifications == 0 && b.borrowed);
                CHECK(co_async_notify(&r->frame, 6) == CO_STATE);
                CHECK(co_async_notify(&r->frame, 7) == CO_OK);
                CHECK(mc_read_cancel(r) == MSG_CANCELLED);
                CHECK(b.status == MSG_CANCELLED && b.progress == 1 &&
                      out[0] == 0x5a);
            }
            else if (mode == 1)
            {
                CHECK(status == MSG_OK && b.status == MSG_OK &&
                      b.progress == 3);
                CHECK(out[0] == 'a' && out[1] == 'b' && out[2] == 'c' &&
                      out[3] == 0x5a);
            }
            else
            {
                CHECK(status == MSG_FAILED && b.status == MSG_FAILED &&
                      b.progress == 2 && out[0] == 0x5a);
            }
            CHECK(b.arms == 1 && b.disarms == 1 && b.cleanups == 1 &&
                  b.notifications == 1);
            if (!release)
            {
                CHECK(mc_read_cancel(r) == MSG_INVALID &&
                      mc_read_poll(r) == MSG_INVALID);
                free(r);
            }
        }
    }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (strcmp(argv[1], "publish") == 0)
    {
        publish();
    }
    else if (strcmp(argv[1], "async") == 0)
    {
        async_read();
    }
    else
    {
        CHECK(0);
    }
    return 0;
}
