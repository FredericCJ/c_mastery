#include "services.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                           \
    do                                                     \
    {                                                      \
        ++checks;                                          \
        if (!(x))                                          \
        {                                                  \
            printf("SERVICE-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                      \
        }                                                  \
    } while (0)
static unsigned checks;

struct sink
{
    unsigned char b[128];
    size_t used, quota;
    enum fr_status status;
};

static struct fr_progress write_sink(void *p, const unsigned char *b, size_t n)
{
    struct sink *s = p;
    size_t k = n < s->quota ? n : s->quota;
    if (k > sizeof s->b - s->used)
    {
        return (struct fr_progress){FR_IO, 0};
    }
    for (size_t i = 0; i < k; ++i)
    {
        s->b[s->used++] = b[i];
    }
    return (struct fr_progress){s->status, k};
}

int main(void)
{
    struct sink sink = {{0}, 0, 2, FR_BLOCKED};
    struct fr_service s;
    unsigned char bytes[24];
    for (unsigned i = 0; i < 24; ++i)
    {
        bytes[i] = (unsigned char)(i + 1);
    }
    CHECK(fr_service_init(&s, write_sink, &sink) == FR_OK);
    CHECK(fr_service_begin(&s, bytes, 24) == FR_NOT_READY);
    void *out = bytes;
    CHECK(fr_heap_extend(&s, 1, &out) == FR_UNSUPPORTED && out == bytes);
    CHECK(fr_service_ready(&s) == FR_OK);
    CHECK(fr_service_begin(&s, bytes, 25) == FR_RANGE);
    CHECK(fr_service_begin(&s, bytes, 24) == FR_OK);
    CHECK(fr_service_begin(&s, bytes, 1) == FR_BLOCKED);
    for (unsigned i = 0; i < 12; ++i)
    {
        CHECK(fr_service_pump(&s) == (i == 11 ? FR_OK : FR_BLOCKED));
    }
    CHECK(sink.used == 24 && !memcmp(sink.b, bytes, 24));
    sink.quota = 0;
    CHECK(fr_service_begin(&s, bytes, 1) == FR_OK);
    CHECK(fr_service_pump(&s) == FR_BLOCKED && s.offset == 0);
    sink.status = FR_IO;
    CHECK(fr_service_pump(&s) == FR_IO && !s.active);
    sink.status = FR_OK;
    CHECK(fr_service_begin(&s, bytes, 1) == FR_OK);
    CHECK(fr_service_pump(&s) == FR_IO);
    CHECK(fr_heap_extend(&s, SIZE_MAX, &out) == FR_UNSUPPORTED && out == bytes);
    for (size_t used = 0; used <= 64; ++used)
    {
        for (size_t align = 1; align <= _Alignof(max_align_t); align *= 2)
        {
            for (size_t n = 1; n <= 65; ++n)
            {
                struct fr_arena a = {{0}, used};
                void *p = bytes;
                size_t start = (used + align - 1) / align * align;
                enum fr_status rc = fr_allocate(&a, n, align, &p);
                if (start + n <= 64)
                {
                    CHECK(rc == FR_OK && p == a.bytes + start &&
                          a.used == start + n && (uintptr_t)p % align == 0);
                }
                else
                {
                    CHECK(rc == FR_EXHAUSTED && p == bytes && a.used == used);
                }
            }
        }
    }
    struct fr_arena arena = {{0}, 0};
    CHECK(fr_allocate(&arena, SIZE_MAX, 8, &out) == FR_EXHAUSTED &&
          out == bytes);
    CHECK(fr_allocate(&arena, 1, 3, &out) == FR_ARGUMENT);
    CHECK(fr_allocate(&arena, 0, 1, &out) == FR_ARGUMENT);
    unsigned char ram[128], rom[32], original[128];
    for (unsigned i = 0; i < 32; ++i)
    {
        rom[i] = (unsigned char)(i + 33);
    }
    memset(original, 0xa5, 128);
    struct fr_start_plan plan = {{8, 24}, {24, 40}, {40, 48}, {64, 128}, 4};
    for (size_t cut = 0; cut <= 32; ++cut)
    {
        memcpy(ram, original, 128);
        CHECK(fr_startup(ram, 128, rom, 32, &plan, cut) ==
              (cut < 32 ? FR_NOT_READY : FR_OK));
        CHECK(!memcmp(ram + 40, original + 40, 8));
        CHECK(fr_startup(ram, 128, rom, 32, &plan, SIZE_MAX) == FR_OK);
        for (unsigned i = 0; i < 128; ++i)
        {
            CHECK(ram[i] == (i >= 8 && i < 24    ? rom[4 + i - 8]
                             : i >= 24 && i < 40 ? 0
                                                 : 0xa5));
        }
    }
    struct fr_start_plan bad = plan;
    bad.bss.begin = 16;
    memcpy(ram, original, 128);
    CHECK(fr_startup(ram, 128, rom, 32, &bad, 0) == FR_OVERLAP &&
          !memcmp(ram, original, 128));
    bad = plan;
    bad.load = SIZE_MAX;
    CHECK(fr_startup(ram, 128, rom, 32, &bad, 0) == FR_RANGE &&
          !memcmp(ram, original, 128));
    bad = plan;
    bad.stack.end = 127;
    CHECK(fr_startup(ram, 128, rom, 32, &bad, 0) == FR_ARGUMENT &&
          !memcmp(ram, original, 128));
    struct fr_tasks tasks = {0};
    struct fr_handle handles[4], old;
    unsigned value = 99;
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(fr_task_create(&tasks, 10 + i, &handles[i]) == FR_OK);
    }
    CHECK(fr_task_create(&tasks, 99, &old) == FR_EXHAUSTED);
    s.ready = 0;
    CHECK(fr_task_hook(&tasks, handles[0], &s) == FR_NOT_READY);
    CHECK(fr_service_ready(&s) == FR_OK);
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(fr_task_hook(&tasks, handles[i], &s) == FR_OK);
        CHECK(fr_task_hook(&tasks, handles[i], &s) == FR_ARGUMENT);
        CHECK(fr_task_read(&tasks, handles[i], &value) == FR_OK &&
              value == 10 + i);
    }
    old = handles[1];
    CHECK(fr_task_destroy(&tasks, old) == FR_OK);
    CHECK(fr_task_create(&tasks, 71, &handles[1]) == FR_OK);
    CHECK(fr_task_read(&tasks, old, &value) == FR_ARGUMENT);
    CHECK(fr_task_read(&tasks, handles[1], &value) == FR_OK && value == 71);
    CHECK(fr_task_destroy(&tasks, handles[1]) == FR_OK);
    tasks.slot[1].generation = UINT_MAX;
    CHECK(fr_task_create(&tasks, 99, &old) == FR_EXHAUSTED);
    printf("SERVICE-PASS:%u\n", checks);
    return 0;
}
