#include "cmk_abi.h"
static unsigned char data[4096];
static volatile unsigned anchor = 1;
#ifdef USER_STORAGE
extern int storage_test(int verify);
#endif
static void finish(int code)
{
    cmk_call(CMK_EXIT, (uint64_t)code, 0, 0);
    for (;;)
    {
    }
}

static void print(const char *s)
{
    size_t n = 0;
    while (s[n])
        n++;
    cmk_write(s, n);
}

static void check_at(int ok, unsigned line)
{
    if (!ok)
    {
        print("FAIL user-contract line ");
        char b[12];
        unsigned n = 0;
        do
        {
            b[n++] = (char)(48 + line % 10);
            line /= 10;
        } while (line);
        while (n)
            cmk_write(b + --n, 1);
        print("\n");
        finish(9);
    }
}

#define check(x) check_at((x), __LINE__)

static int equal(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return *a == *b;
}

__attribute__((section(".text.start"), noreturn)) void _start(void)
{
    if (!anchor)
        finish(10);
#if defined(USER_FAULT)
    *(volatile unsigned char *)(uintptr_t)0x100000 = 42;
    finish(11);
#elif defined(USER_WORKER)
    volatile uint64_t turns = 0;
    unsigned char id = (unsigned char)cmk_call(CMK_PID, 0, 0, 0);
    volatile unsigned char *owned = data;
    owned[0] = id;
    while (cmk_call(CMK_TICKS, 0, 0, 0) < 8)
    {
        for (unsigned i = 0; i < 10000; i++)
            turns++;
        check(owned[0] == id);
    }
    check(turns > 0);
    print("PASS preempted-worker\n");
    finish(0);
#elif defined(USER_SHELL)
    print("CMK shell: help, echo TEXT, exit\n");
    char line[64];
    unsigned used = 0;
    print("cmk> ");
    for (;;)
    {
        char c;
        if (cmk_call(CMK_READ, (uintptr_t)&c, 0, 0) == 1)
        {
            if (c == '\r' || c == '\n')
            {
                line[used] = 0;
                if (equal(line, "exit"))
                    finish(0);
                if (equal(line, "help"))
                    print("help | echo TEXT | exit\n");
                else if (used >= 5 && line[0] == 'e' && line[1] == 'c' &&
                         line[2] == 'h' && line[3] == 'o' && line[4] == ' ')
                {
                    print(line + 5);
                    print("\n");
                }
                else
                    print("unknown command\n");
                used = 0;
                print("cmk> ");
            }
            else if (used < sizeof line - 1)
                line[used++] = c;
            else
            {
                used = 0;
                print("line too long\ncmk> ");
            }
        }
        else
            cmk_call(CMK_YIELD, 0, 0, 0);
    }
#else
    check(cmk_call(CMK_WRITE, 0x100000, 4, 0) == -1);
    check(cmk_call(CMK_WRITE, UINT64_MAX - 1, 8, 0) == -1);
    check(cmk_call(999, 0, 0, 0) == -1);
    unsigned char *heap = cmk_alloc(2);
    check(heap != NULL);
    for (unsigned i = 0; i < 8192; i++)
        check(heap[i] == 0);
    heap[8191] = 71;
    check(cmk_call(CMK_ALLOC, 17, 0, 0) == -1);
    check(cmk_call(CMK_BLOCK_COUNT, CMK_DISK_HANDLE + 1, 0, 0) == -1);
    check(cmk_call(CMK_BLOCK_COUNT, CMK_DISK_HANDLE, 0, 0) > 8);
    int verify = (int)cmk_call(CMK_BOOT_MODE, 0, 0, 0);
    void *handle = (void *)(uintptr_t)CMK_DISK_HANDLE;
    if (!verify)
    {
        for (unsigned i = 0; i < 4096; i++)
            data[i] = (unsigned char)(i * 17u + 3u);
        check(cmk_block_write(handle, 3, data) == 0);
        check(cmk_block_flush(handle) == 0);
    }
    for (unsigned i = 0; i < 4096; i++)
        data[i] = 0;
    check(cmk_block_read(handle, 3, data) == 0);
    for (unsigned i = 0; i < 4096; i++)
        check(data[i] == (unsigned char)(i * 17u + 3u));
    check(cmk_block_read(handle, UINT32_MAX, data) == -1);
    check(cmk_block_read(handle, 3, (void *)(uintptr_t)0x100000) == -1);
#ifdef USER_STORAGE
    int storage_status = storage_test(verify);
    if (storage_status)
    {
        char msg[] = {'S', 'T', 'O', 'R', 'E', ' ', (char)(48 + storage_status),
                      '\n'};
        cmk_write(msg, sizeof msg);
    }
    check(storage_status == 0);
#endif
    check(cmk_call(CMK_CLOSE, CMK_DISK_HANDLE, 0, 0) == 0);
    check(cmk_block_flush(handle) == -1);
    print(verify ? "PASS persistent-read-after-reboot\n"
                 : "PASS block-write-flush-read\n");
    print("PASS user-copy-handles-heap\n");
    finish(0);
#endif
}
