#include "contracts.h"
#include "kf_abi.h"
#if defined(USER_WORKER)
static unsigned char data[8192] __attribute__((aligned(4096)));
#endif
static volatile unsigned initialized_anchor = 1;
volatile uint64_t kf_remaining = 12000000, kf_turns;
extern unsigned kf_register_loop(void);
extern void kf_skip_guard(void), kf_probe_stack(void);

static _Noreturn void finish(int code)
{
    cmk_call(CMK_EXIT, (uint64_t)code, 0, 0);
    for (;;)
    {
    }
}

static void print(const char *s)
{
    size_t n = 0;
    while (s[n] && n < 512)
    {
        ++n;
    }
    cmk_write(s, n);
}

static void check(int okay)
{
    if (!okay)
    {
        print("KF:FAIL:USER\n");
        finish(9);
    }
}

static void hex(uint64_t x)
{
    char b[17];
    for (unsigned i = 0; i < 16; ++i)
    {
        b[i] = "0123456789abcdef"[(x >> (60 - 4 * i)) & 15];
    }
    b[16] = '\n';
    cmk_write(b, 17);
}

__attribute__((section(".text.start"), noreturn)) void _start(void)
{
    if (!initialized_anchor)
    {
        finish(10);
    }
#if defined(USER_FAULT)
    if (KF_CASE == KF_USER_DIVIDE || KF_CASE == KF_NESTED ||
        KF_CASE == KF_BUDGET)
    {
        __asm__ volatile(
            "xor %%edx,%%edx; mov $1,%%eax; xor %%ecx,%%ecx; div %%ecx" ::
                : "rax", "rcx", "rdx", "cc");
        finish(10);
    }
    if (KF_CASE == KF_NX)
    {
        unsigned char *p = cmk_alloc(1);
        check(p != 0);
        p[0] = 0xc3;
        __asm__ volatile("call *%0" ::"r"(p) : "memory");
        finish(10);
    }
    if (KF_CASE == KF_GUARD)
    {
        *(volatile unsigned char *)(uintptr_t)0x401df000 = 24;
        finish(10);
    }
    if (KF_CASE == KF_SKIP_GUARD)
    {
        kf_skip_guard();
        print("KF:PASS:SKIPPED_GUARD_LANDED_MAPPED\n");
        finish(0);
    }
    if (KF_CASE == KF_PROBE)
    {
        kf_probe_stack();
        finish(10);
    }
    *(volatile unsigned char *)(uintptr_t)KF_SENTINEL_VA = 42;
    print("KF:PASS:DECLARED_FLAW_CHANGED_SENTINEL\n");
    finish(KF_CASE == KF_FLAWED ? 0 : 11);
#elif defined(USER_WORKER)
    uint64_t id = (uint64_t)cmk_call(CMK_PID, 0, 0, 0);
    volatile unsigned char *p = data;
    p[0] = (unsigned char)id;
    check(cmk_call(CMK_BLOCK_COUNT, CMK_DISK_HANDLE, 0, 0) == -1);
    print("KF:WORKER_START:pid=");
    hex(id);
    print("same_va=");
    hex((uintptr_t)data);
    unsigned failed = kf_register_loop();
    if (failed)
    {
        print("KF:FAIL:SAVED_REGISTER=");
        hex(failed);
        finish(9);
    }
    check(p[0] == id && kf_turns == 12000000);
    print("KF:PASS:WORKER_GPRS_PRIVATE_MEMORY:pid=");
    hex(id);
    print("ticks=");
    hex((uint64_t)cmk_call(CMK_TICKS, 0, 0, 0));
    finish(0);
#elif defined(USER_SHELL)
    struct kf_line input = {0};
    char line[64];
    unsigned length;
    print("KF:SHELL_READY\nkf> ");
    for (;;)
    {
        unsigned char c;
        int64_t n = cmk_call(CMK_READ, (uintptr_t)&c, 0, 0);
        if (n == 0)
        {
            cmk_call(CMK_YIELD, 0, 0, 0);
            continue;
        }
        check(n == 1);
        if (c == 4)
        {
            print("KF:SHELL_PARTIAL_DISCARDED=");
            hex(input.used);
            finish(0);
        }
        int state = kf_line(&input, c, line, &length);
        if (!state)
        {
            continue;
        }
        if (state == 2)
        {
            print("KF:SHELL_DISCARDED_OVERLONG\n");
        }
        else if (length == 4 && line[0] == 'e' && line[1] == 'x' &&
                 line[2] == 'i' && line[3] == 't')
        {
            finish(0);
        }
        else if (length == 4 && line[0] == 'h' && line[1] == 'e' &&
                 line[2] == 'l' && line[3] == 'p')
        {
            print("KF:SHELL_HELP:help echo stats exit\n");
        }
        else if (length >= 5 && line[0] == 'e' && line[1] == 'c' &&
                 line[2] == 'h' && line[3] == 'o' && line[4] == ' ')
        {
            print("KF:SHELL_ECHO:");
            cmk_write(line + 5, length - 5);
            print("\n");
        }
        else if (length == 5 && line[0] == 's' && line[1] == 't' &&
                 line[2] == 'a' && line[3] == 't' && line[4] == 's')
        {
            struct kf_stats value = {0};
            check(cmk_call(KF_STATS, (uintptr_t)&value, 1, sizeof value) == 0);
            struct kf_stats saved = value;
            check(cmk_call(KF_STATS, (uintptr_t)&value, 9, sizeof value) == -1);
            check(value.free_frames == saved.free_frames &&
                  value.pid == saved.pid && value.version == saved.version);
            print("KF:SHELL_STATS:bounded-copied-v1:refused-v9-unchanged\n");
        }
        else
        {
            print("KF:SHELL_UNKNOWN\n");
        }
        print("kf> ");
    }
#else
    struct kf_stats stats = {0};
    check(cmk_call(KF_STATS, (uintptr_t)&stats, 1, sizeof stats) == 0 &&
          stats.version == 1 && stats.size == sizeof stats);
    struct kf_stats before = stats;
    check(cmk_call(KF_STATS, (uintptr_t)&stats, 2, sizeof stats) == -1);
    check(stats.pid == before.pid && stats.ticks == before.ticks &&
          stats.free_frames == before.free_frames);
    check(cmk_call(CMK_WRITE, 0x100000, 4, 0) == -1);
    check(cmk_call(CMK_WRITE, UINT64_MAX - 1, 8, 0) == -1);
    check(cmk_call(999, 0, 0, 0) == -1);
    check(cmk_call(CMK_ALLOC, 17, 0, 0) == -1);
    unsigned char *p = cmk_alloc(2);
    if (KF_CASE == 25)
    {
        check(!p);
        check(cmk_call(KF_STATS, (uintptr_t)&stats, 1, sizeof stats) == 0 &&
              stats.free_frames == before.free_frames);
        print("KF:PASS:CHANGED_ALLOC_LIMIT_ONE\n");
        finish(0);
    }
    check(p != 0);
    for (unsigned i = 0; i < 8192; ++i)
    {
        check(p[i] == 0);
    }
    p[8191] = 71;
    if (KF_CASE == 22)
    {
        for (unsigned fail = 0; fail < 16; ++fail)
        {
            check(cmk_call(KF_STATS, (uintptr_t)&before, 1, sizeof before) ==
                  0);
            check(cmk_call(KF_ALLOC_SET, fail, 0, 0) == 0);
            check(!cmk_alloc(16));
            check(cmk_call(KF_STATS, (uintptr_t)&stats, 1, sizeof stats) == 0 &&
                  stats.free_frames == before.free_frames && p[8191] == 71);
        }
        check(cmk_call(KF_ALLOC_SET, 16, 0, 0) == 0);
        unsigned char *q = cmk_alloc(16);
        check(q == p + 8192);
        for (unsigned i = 0; i < 16 * 4096; ++i)
        {
            check(q[i] == 0);
        }
        print("KF:PASS:ACTUAL_ALLOC_16_FAILURE_PREFIXES_ZERO_SUCCESS\n");
    }
    if (KF_CASE != 22)
    {
        check(cmk_call(CMK_WRITE, (uintptr_t)p + 8191, 2, 0) == -1);
    }
    check(cmk_call(CMK_WRITE, (uintptr_t)p, 0, 0) == 0);
    check(cmk_call(CMK_WRITE, 0x40200000, 0, 0) == -1);
    check(cmk_call(CMK_READ, (uintptr_t)_start, 0, 0) == -1);
    void *h = (void *)(uintptr_t)CMK_DISK_HANDLE;
    check(cmk_call(CMK_BLOCK_COUNT, CMK_DISK_HANDLE, 0, 0) == 1024);
    check(cmk_call(CMK_BLOCK_COUNT, UINT64_C(0x100010001), 0, 0) == -1);
    check(cmk_call(KF_STATS, (uintptr_t)&before, 1, sizeof before) == 0);
    check(cmk_block_read(h, 3, (void *)(uintptr_t)0x401defff) == -1);
    check(cmk_call(KF_STATS, (uintptr_t)&stats, 1, sizeof stats) == 0);
    check(before.device_submissions == stats.device_submissions &&
          before.free_frames == stats.free_frames);
    check(cmk_call(CMK_BLOCK_COUNT, CMK_DISK_HANDLE, 0, 0) == 1024);
    if (KF_CASE == KF_LATENCY)
    {
        check(cmk_block_read(h, 0, p) == 0);
        for (unsigned i = 0; i < 4096; ++i)
        {
            check(p[i] == 0);
        }
    }
    check(cmk_call(CMK_CLOSE, CMK_DISK_HANDLE, 0, 0) == 0);
    check(cmk_block_flush(h) == -1);
    print(
        "KF:PASS:SYSCALL_PREFLIGHT_NO_CONSUMPTION\nKF:PASS:VERSIONED_STATS\n");
    if (KF_CASE == KF_LATENCY)
    {
        check(cmk_block_read(h, 0, p) == -1);
        print("KF:USER_POLL_BEGIN\n");
        for (unsigned i = 0; i < KF_POLL_ITER; ++i)
        {
            __asm__ volatile("pause");
        }
        print("KF:USER_POLL_END\n");
        check(cmk_call(KF_POLL, KF_POLL_ITER, 0, 0) >= 0);
    }
    finish(0);
#endif
}
