#include "cmk_contract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

static void w16(unsigned char *p, unsigned n)
{
    p[0] = (unsigned char)n;
    p[1] = (unsigned char)(n >> 8);
}

static void w32(unsigned char *p, uint32_t n)
{
    for (unsigned i = 0; i < 4; i++)
        p[i] = (unsigned char)(n >> (8 * i));
}

static void w64(unsigned char *p, uint64_t n)
{
    for (unsigned i = 0; i < 8; i++)
        p[i] = (unsigned char)(n >> (8 * i));
}

static size_t boot(unsigned char *p)
{
    memset(p, 0, 128);
    w32(p, 96);
    w32(p + 8, 3);
    w32(p + 12, 21);
    w32(p + 16, 0x200000);
    w32(p + 20, 0x210000);
    memcpy(p + 24, "init", 5);
    w32(p + 32, 6);
    w32(p + 36, 56);
    w32(p + 40, 40);
    w64(p + 48, 0);
    w64(p + 56, 0x400000);
    w32(p + 64, 1);
    w32(p + 88, 0);
    w32(p + 92, 8);
    return 96;
}

static int test_boot(void)
{
    unsigned char p[128];
    struct cmk_boot b = {0};
    size_t n = boot(p);
    CHECK(cmk_boot_parse(p, n, &b) == 0);
    CHECK(b.entry_size == 40 && b.map_bytes == 40 && b.module_end == 0x210000);
    for (size_t i = 0; i < n; i++)
        CHECK(cmk_boot_parse(p, i, &b) != 0);
    w32(p + 12, 7);
    CHECK(cmk_boot_parse(p, n, &b) != 0);
    boot(p);
    w64(p + 48, UINT64_MAX - 3);
    w64(p + 56, 8);
    CHECK(cmk_boot_parse(p, n, &b) != 0);
    boot(p);
    w32(p + 40, 25);
    CHECK(cmk_boot_parse(p, n, &b) != 0);
    boot(p);
    w32(p + 12, UINT32_MAX);
    CHECK(cmk_boot_parse(p, n, &b) != 0);
    return 0;
}

static int test_frames(void)
{
    unsigned char p[128];
    struct cmk_boot b;
    struct cmk_frames f;
    boot(p);
    CHECK(cmk_boot_parse(p, 96, &b) == 0);
    cmk_frames_init(&f, &b, 0x300000);
    uint32_t first = cmk_frame_take(&f);
    CHECK(first == 0x300000);
    CHECK(cmk_frame_put(&f, first + 1) != 0);
    CHECK(cmk_frame_put(&f, 0x1000) != 0);
    CHECK(cmk_frame_put(&f, first) == 0);
    CHECK(cmk_frame_put(&f, first) != 0);
    unsigned count = 0;
    while (cmk_frame_take(&f))
        count++;
    CHECK(count == 256);
    return 0;
}

static void elf(unsigned char *p)
{
    memset(p, 0, 8192);
    memcpy(p, "\177ELF", 4);
    p[4] = 2;
    p[5] = 1;
    p[6] = 1;
    w16(p + 16, 2);
    w16(p + 18, 62);
    w32(p + 20, 1);
    w64(p + 24, CMK_USER_BASE);
    w64(p + 32, 64);
    w16(p + 52, 64);
    w16(p + 54, 56);
    w16(p + 56, 1);
    w32(p + 64, 1);
    w32(p + 68, 5);
    w64(p + 72, 4096);
    w64(p + 80, CMK_USER_BASE);
    w64(p + 96, 1);
    w64(p + 104, 4096);
    w64(p + 112, 4096);
    p[4096] = 0xc3;
}

static int test_elf(void)
{
    unsigned char p[8192];
    struct cmk_elf e;
    elf(p);
    CHECK(cmk_elf_parse(p, sizeof p, &e) == 0);
    CHECK(e.count == 1 && e.segments[0].filesz == 1 &&
          e.segments[0].memsz == 4096);
    for (size_t i = 0; i <= 4096; i++)
        CHECK(cmk_elf_parse(p, i, &e) != 0);
    w32(p + 68, 7);
    CHECK(cmk_elf_parse(p, sizeof p, &e) != 0);
    elf(p);
    w64(p + 104, 0);
    CHECK(cmk_elf_parse(p, sizeof p, &e) != 0);
    elf(p);
    w64(p + 80, UINT64_MAX - 4);
    CHECK(cmk_elf_parse(p, sizeof p, &e) != 0);
    elf(p);
    w16(p + 56, 2);
    memcpy(p + 120, p + 64, 56);
    CHECK(cmk_elf_parse(p, sizeof p, &e) != 0);
    elf(p);
    w32(p + 64, 3);
    CHECK(cmk_elf_parse(p, sizeof p, &e) != 0);
    return 0;
}

static int test_archive(void)
{
    unsigned char p[64] = {0};
    const unsigned char *d = 0;
    size_t n = 0;
    memcpy(p, "CMKA", 4);
    w32(p + 4, 1);
    w32(p + 8, 1);
    w32(p + 12, 64);
    memcpy(p + 16, "init", 5);
    w32(p + 40, 56);
    w32(p + 44, 8);
    memcpy(p + 56, "payload!", 8);
    CHECK(cmk_archive_find(p, 64, "init", &d, &n) == 0);
    CHECK(n == 8 && memcmp(d, "payload!", 8) == 0);
    CHECK(cmk_archive_find(p, 64, "other", &d, &n) != 0);
    char unbounded_name[24];
    memset(unbounded_name, 'x', sizeof unbounded_name);
    CHECK(cmk_archive_find(p, 64, unbounded_name, &d, &n) != 0);
    for (size_t i = 0; i < 64; i++)
        CHECK(cmk_archive_find(p, i, "init", &d, &n) != 0);
    w32(p + 40, 55);
    CHECK(cmk_archive_find(p, 64, "init", &d, &n) != 0);
    return 0;
}

static int test_handles(void)
{
    uint32_t h = cmk_handle(0, 1);
    CHECK(h == 0x10001);
    CHECK(cmk_handle_valid(h, 0, 1, 1));
    CHECK(!cmk_handle_valid(h, 0, 2, 1));
    CHECK(!cmk_handle_valid(h, 0, 1, 0));
    CHECK(!cmk_handle_valid(h, 1, 1, 1));
    CHECK(cmk_handle(16, 1) == 0);
    CHECK(cmk_handle(0, 0) == 0);
    CHECK(cmk_handle(0, 65536) == 0);
    return 0;
}

static int test_ranges(void)
{
    CHECK(cmk_user_range(CMK_USER_BASE, 4096));
    CHECK(cmk_user_range(CMK_USER_LIMIT - 1, 1));
    CHECK(!cmk_user_range(CMK_USER_LIMIT - 1, 2));
    CHECK(!cmk_user_range(0, 1));
    CHECK(!cmk_user_range(CMK_USER_BASE, UINT64_MAX));
    CHECK(!cmk_user_range(CMK_USER_LIMIT, 0));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    int r;
    if (!strcmp(argv[1], "boot"))
        r = test_boot();
    else if (!strcmp(argv[1], "frames"))
        r = test_frames();
    else if (!strcmp(argv[1], "elf"))
        r = test_elf();
    else if (!strcmp(argv[1], "archive"))
        r = test_archive();
    else if (!strcmp(argv[1], "handles"))
        r = test_handles();
    else if (!strcmp(argv[1], "ranges"))
        r = test_ranges();
    else
        return 2;
    if (!r)
        printf("PASS %s\n", argv[1]);
    return r;
}
