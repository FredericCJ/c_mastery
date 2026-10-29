#include "release.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                      \
    do                                                \
    {                                                 \
        if (!(x))                                     \
        {                                             \
            printf("DT_CHECK:%u:%s\n", __LINE__, #x); \
            return 1;                                 \
        }                                             \
    } while (0)

struct du_mask
{
    uint32_t mask;
    unsigned entered, left;
};

static uint32_t enter(void *v)
{
    struct du_mask *m = v;
    uint32_t old = m->mask;
    m->mask = 1;
    m->entered++;
    return old;
}

static void leave(void *v, uint32_t old)
{
    struct du_mask *m = v;
    m->mask = old;
    m->left++;
}

int main(void)
{
    struct dt_trace q = {0};
    q.next = 10;
    struct du_mask m = {1, 0, 0};
    struct dt_critical port = {&m, enter, leave};
    struct dt_record r = {99, 99, 99, 99};
    CHECK(dt_irq_emit(&q, port, 1, 100, 10) == DT_OK && m.mask == 1 &&
          m.entered == m.left);
    CHECK(dt_irq_emit(&q, port, 2, 101, 11) == DT_OK);
    CHECK(dt_irq_emit(&q, port, 3, 102, 12) == DT_OK);
    CHECK(dt_irq_emit(&q, port, 4, 103, 13) == DT_FULL && q.dropped == 1 &&
          q.next == 14);
    CHECK(dt_irq_take(&q, port, &r) == DT_OK && r.sequence == 10);
    CHECK(dt_take(&q, &r) == DT_OK && r.sequence == 11);
    CHECK(dt_take(&q, &r) == DT_OK && r.sequence == 12);
    CHECK(dt_emit(&q, 5, 104, 14) == DT_OK);
    CHECK(dt_take(&q, &r) == DT_OK && r.sequence == 14);
    CHECK(dt_take(&q, &r) == DT_EMPTY && r.sequence == 14);
    q.next = UINT32_MAX - 1;
    CHECK(dt_emit(&q, 1, 0, 0) == DT_OK);
    CHECK(dt_emit(&q, 1, 0, 0) == DT_EXHAUSTED);
    CHECK(dt_take(&q, &r) == DT_OK && r.sequence == UINT32_MAX - 1);
    q = (struct dt_trace){0};
    q.count = 3;
    q.dropped = UINT32_MAX;
    CHECK(dt_emit(&q, 1, 0, 0) == DT_EXHAUSTED && q.next == 0);
    struct dt_interval v[2] = {{250, 4, 1000000, 8}, {4, 24, 2000000, 8}};
    struct dt_duration duration = {9, 9};
    CHECK(dt_duration(v, 2, &duration) == DT_OK && duration.lower_ns == 20000 &&
          duration.upper_ns == 20000);
    v[0] = (struct dt_interval){0, 1, 3, 8};
    CHECK(dt_duration(v, 1, &duration) == DT_OK &&
          duration.lower_ns == 333333333 && duration.upper_ns == 333333334);
    v[0].width = 0;
    CHECK(dt_duration(v, 1, &duration) == DT_INVALID &&
          duration.lower_ns == 333333333);
    struct dt_interval overflow[8];
    for (unsigned i = 0; i < 8; i++)
        overflow[i] = (struct dt_interval){0, UINT32_MAX, 1, 32};
    CHECK(dt_duration(overflow, 8, &duration) == DT_RANGE);
    struct dt_image image = {9, 9};
    CHECK(dt_image(0x08000000, 1024, 256, &image) == DT_OK &&
          image.vector == 0x08000100 && image.end == 0x08000400);
    CHECK(dt_image(0xfffffffc, 8, 0, &image) == DT_RANGE &&
          image.vector == 0x08000100);
    CHECK(dt_image(0, 8, 4, &image) == DT_INVALID);
    struct dt_ram ram;
    memset(&ram, 0xa5, sizeof ram);
    const unsigned char init[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    CHECK(dt_startup(&ram, init, 0, 17) == DT_OK && !ram.crash.valid);
    for (unsigned i = 0; i < 8; i++)
        CHECK(ram.data[i] == init[i] && ram.bss[i] == 0);
    CHECK(dt_record(&ram.crash, 17, 42) == DT_OK);
    CHECK(dt_startup(&ram, init, 1, 17) == DT_OK && ram.crash.code == 42);
    ram.crash.version = 2;
    CHECK(dt_startup(&ram, init, 1, 17) == DT_OK && !ram.crash.valid);
    CHECK(dt_record(&ram.crash, 17, 42) == DT_OK);
    CHECK(dt_startup(&ram, init, 1, 18) == DT_OK && !ram.crash.valid);
    CHECK(dt_record(&ram.crash, 17, 42) == DT_OK);
    ram.crash.check ^= 1;
    CHECK(dt_startup(&ram, init, 1, 17) == DT_OK && !ram.crash.valid);
    puts("DT_PASS:core");
    return 0;
}
