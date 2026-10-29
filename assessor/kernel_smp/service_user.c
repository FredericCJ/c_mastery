#include "cmk2_abi.h"
#include "cvm.h"
#include "store.h"
#include "store_host.h"
static store database;
static struct cvm_cvm vm;
static struct cvm_program program;
static struct cvm_value result;

void *memcpy(void *d, const void *s, size_t n)
{
    unsigned char *p = d;
    const unsigned char *q = s;
    while (n--)
    {
        *p++ = *q++;
    }
    return d;
}

void *memset(void *d, int c, size_t n)
{
    unsigned char *p = d;
    while (n--)
    {
        *p++ = (unsigned char)c;
    }
    return d;
}

void *memmove(void *d, const void *s, size_t n)
{
    unsigned char *p = d;
    const unsigned char *q = s;
    /* This adapter is restricted to the fixed flat-address x86-64 guest. */
    if ((uintptr_t)p < (uintptr_t)q)
    {
        while (n--)
        {
            *p++ = *q++;
        }
    }
    else
    {
        while (n)
        {
            n--;
            p[n] = q[n];
        }
    }
    return d;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *p = a, *q = b;
    while (n--)
    {
        if (*p != *q)
        {
            return *p < *q ? -1 : 1;
        }
        p++;
        q++;
    }
    return 0;
}

static int read_page(void *ctx, uint32_t p, void *out)
{
    if (p >= STORE_IMAGE_PAGES)
    {
        return -1;
    }
    return cmk2_block(ctx, 0, p + 128, out);
}

static int write_page(void *ctx, uint32_t p, const void *in)
{
    if (p >= STORE_IMAGE_PAGES)
    {
        return -1;
    }
    return cmk2_block(ctx, 1, p + 128, (void *)in);
}

static uint32_t requests, replies;

static int flush_page(void *ctx)
{
    return cmk2_block(ctx, 4, 0, 0);
}

static void print(const char *p)
{
    size_t n = 0;
    while (p[n])
    {
        n++;
    }
    cmk_write(p, n);
}

static void finish(unsigned status)
{
    cmk_call(CMK_EXIT, status, 0, 0);
    for (;;)
    {
    }
}

#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            char msg[] = {'0' + (__LINE__ / 100) % 10,                     \
                          '0' + (__LINE__ / 10) % 10, '0' + __LINE__ % 10, \
                          '\n', 0};                                        \
            print("FAIL owner-service line ");                             \
            print(msg);                                                    \
            finish(__LINE__);                                              \
        }                                                                  \
    } while (0)

static int client_host(void *unused, uint8_t service, int32_t input,
                       int32_t *output)
{
    (void)unused;
    if (input < 0 || input > 0xffffff || service > 2)
    {
        return STORE_INVALID;
    }
    if (cmk_call(CMK2_CHANNEL_SEND, requests,
                 ((uint32_t)service << 24) | (uint32_t)input, 0))
    {
        return STORE_IO;
    }
    int64_t response = cmk_call(CMK2_CHANNEL_RECV, replies, 0, 0);
    if (response < 0)
    {
        return STORE_IO;
    }
    if (response & (1u << 30))
    {
        return STORE_INDETERMINATE;
    }
    *output = (int32_t)response;
    return 0;
}

void _start(void)
{
    int verify = (int)cmk_call(CMK_BOOT_MODE, 0, 0, 0);
    store_io io = {(void *)(uintptr_t)CMK_DISK_HANDLE, read_page, write_page,
                   flush_page, STORE_IMAGE_PAGES};
    if (!verify)
    {
        CHECK(store_format(io) == STORE_OK);
    }
    CHECK(store_open(&database, io) == STORE_OK);
    int64_t rq = cmk_call(CMK2_CHANNEL_CREATE, 0, 0, 0),
            rp = cmk_call(CMK2_CHANNEL_CREATE, 0, 0, 0);
    CHECK(rq > 0 && rp > 0);
    requests = (uint32_t)rq;
    replies = (uint32_t)rp;
    int64_t child = cmk2_fork();
    CHECK(child >= 0);
    if (!child)
    {
        CHECK(cmk_call(CMK_BLOCK_COUNT, CMK_DISK_HANDLE, 0, 0) == -1);
        uint8_t bytes[] = {'C', 'V',    'M', '1', 1, 0, 0,       8, 0,     0,
                           0,   OP_INT, 42,  0,   0, 0, OP_HOST, 1, OP_RET};
        if (verify)
        {
            bytes[17] = 2;
        }
        CHECK(cvm_load(bytes, sizeof bytes, &program) == CVM_OK);
        cvm_init(&vm, 32, 128, 1);
        vm.host = client_host;
        vm.host_context = 0;
        enum cvm_status status = cvm_run(&vm, &program, &result);
        if (verify)
        {
            CHECK(status == CVM_OK && result.tag == CVM_INT &&
                  result.integer == 42);
        }
        else
        {
            CHECK(status == CVM_HOST_ERROR);
        }
        print("PASS v2-VM-client-no-disk-capability\n");
        finish(0);
    }
    int64_t request = cmk_call(CMK2_CHANNEL_RECV, requests, 0, 0);
    CHECK(request >= 0);
    unsigned operation = ((unsigned)request >> 24) & 255;
    int32_t output = 0;
    CHECK(cvm_store_host(&database, (uint8_t)operation,
                         (int32_t)(request & 0xffffff), &output) == 0);
    CHECK(cmk_call(CMK2_CHANNEL_SEND, replies,
                   verify ? (uint32_t)output : (1u << 30), 0) == 0);
    CHECK(cmk_call(CMK2_WAIT_PROCESS, (uint64_t)child, 0, 0) == 0);
    CHECK(store_check(&database) == STORE_OK);
    print(verify ? "PASS v2-owner-store-recovery-after-reboot\n"
                 : "PASS v2-owner-store-commit-indeterminate-reply\n");
    finish(0);
}
