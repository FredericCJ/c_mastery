#include "abi.h"
#include "service.h"
#include <string.h>
static store database;
static volatile unsigned image_word;
static unsigned corrupt;

static void done(unsigned n)
{
    cmk_call(CMK_EXIT, n, 0, 0);
    for (;;)
    {
    }
}

static void check(int okay)
{
    if (!okay)
    {
        cmk_write("SF_STORE_CHECK\n", 15);
        done(91);
    }
}

static int read_page(void *context, uint32_t page, void *bytes)
{
    return page >= STORE_IMAGE_PAGES
               ? -1
               : cmk2_block(context, 0, page + 128, bytes);
}

static int write_page(void *context, uint32_t page, const void *bytes)
{
    if (page >= STORE_IMAGE_PAGES)
        return -1;
    if (corrupt && page == 35)
    {
        unsigned char damaged[4096];
        memcpy(damaged, bytes, 4096);
        if (corrupt == 1)
            damaged[100] ^= 0x80;
        else
            memset(damaged + 512, 0, 4096 - 512);
        check(!cmk2_block(context, 1, page + 128, damaged) &&
              !cmk2_block(context, 4, 0, 0));
        return -1;
    }
    return cmk2_block(context, 1, page + 128, (void *)bytes);
}

static int flush(void *context)
{
    return cmk2_block(context, 4, 0, 0);
}

static int64_t receive(uint64_t q, void *p, size_t n, uint64_t *sender)
{
    struct sf_receive r = {(uintptr_t)p, n, 0};
    int64_t status;
    do
    {
        status = cmk_call(SF_RECEIVE, q, (uintptr_t)&r, 0);
    } while (status == SF_RETRY);
    if (sender)
        *sender = r.sender;
    return status;
}

static int64_t send(uint64_t q, const void *p, size_t n)
{
    struct sf_span s = {(uintptr_t)p, n};
    return cmk_call(SF_SEND, q, (uintptr_t)&s, 0);
}

void _start(void)
{
    image_word = 1;
    check(image_word == 1);
    struct sf_bootstrap b;
    while (cmk_call(SF_BOOTSTRAP, (uintptr_t)&b, 0, 0) == SF_FULL)
        cmk_call(CMK_YIELD, 0, 0, 0);
    store_io io = {(void *)(uintptr_t)CMK_DISK_HANDLE, read_page, write_page,
                   flush, STORE_IMAGE_PAGES};
    if (!b.role)
        check(store_format(io) == STORE_OK);
    check(store_open(&database, io) == STORE_OK);
    if (!b.role)
        check(!sdr_initialize(&database) &&
              store_checkpoint(&database) == STORE_OK);
    struct sfs_config config;
    uint64_t sender;
    check(receive(b.request, &config, sizeof config, &sender) ==
              sizeof config &&
          sender == SF_PROCESS_TAG + 65537);
    for (;;)
    {
        struct sfs_request q;
        int64_t n = receive(b.request, &q, sizeof q, &sender);
        if (n == SF_CLOSED)
            break;
        check(n == sizeof q);
        unsigned client = sender == config.clients[0]
                              ? 1
                              : (sender == config.clients[1] ? 2 : 0);
        check(client && q.client == client);
        struct sfs_reply r = {q.session, q.id, SFS_SESSION, 0, 0, {0}};
        if (q.session == b.session)
        {
            if (q.operation == SFS_DAMAGE_UNCOMMITTED)
            {
                check(q.flags == 1 || q.flags == 2);
                check(!store_checkpoint(&database));
                check(!store_begin(&database));
                unsigned char value[4] = {99, 0, 0, 0};
                check(!store_put(&database, "effect", 6, value, 4));
                corrupt = q.flags;
                check(store_commit(&database) == STORE_IO);
                cmk_write("PASS sf-uncommitted-tail-written\n", 33);
                done(8);
            }
            int error = sfs_apply(&database, &q, &r);
            if (error)
                r.status = error;
        }
        if (q.flags == 1 && q.operation == SFS_INCREMENT && !r.status)
        {
            cmk_write("PASS sf-durable-before-lost-reply\n", 34);
            done(7);
        }
        check(!send(config.reply[client - 1], &r, sizeof r));
    }
    check(store_check(&database) == STORE_OK);
    done(0);
}
