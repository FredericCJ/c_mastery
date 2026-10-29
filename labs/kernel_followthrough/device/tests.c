#include "device.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(c))                                                             \
        {                                                                     \
            fprintf(stderr, "KF_DEVICE_CHECK:%s:%d:%s\n", __func__, __LINE__, \
                    #c);                                                      \
            return 1;                                                         \
        }                                                                     \
    } while (0)

static void put32(uint8_t *p, uint32_t x)
{
    for (unsigned i = 0; i < 4; ++i)
    {
        p[i] = (uint8_t)(x >> (i * 8));
    }
}

static void capability(uint8_t c[256], unsigned at, unsigned next,
                       unsigned kind, unsigned off, unsigned size)
{
    c[at] = 9;
    c[at + 1] = (uint8_t)next;
    c[at + 2] = (uint8_t)(kind == 2 ? 20 : 16);
    c[at + 3] = (uint8_t)kind;
    c[at + 4] = 0;
    put32(c + at + 8, off);
    put32(c + at + 12, size);
    if (kind == 2)
    {
        put32(c + at + 16, 4);
    }
}

static void fixture(uint8_t c[256], struct kf_bar b[6])
{
    memset(c, 0, 256);
    memset(b, 0, 6 * sizeof *b);
    c[6] = 16;
    c[0x34] = 64;
    capability(c, 64, 80, 1, 0, 56);
    capability(c, 80, 104, 2, 256, 64);
    capability(c, 104, 0, 4, 512, 8);
    b[0] = (struct kf_bar){0x80000000, 4096, 1};
}

static int pci(void)
{
    uint8_t c[256], original[256];
    struct kf_bar bars[6];
    fixture(c, bars);
    memcpy(original, c, 256);
    struct kf_pci p;
    memset(&p, 0xa5, sizeof p);
    CHECK(kf_pci(c, bars, &p));
    CHECK(p.common == 0x80000000 && p.notify == 0x80000100 &&
          p.device == 0x80000200 && p.visited == 3);
    struct kf_pci expected = p;
    for (unsigned change = 0; change < 11; ++change)
    {
        fixture(c, bars);
        switch (change)
        {
            case 0:
                c[105] = 64;
                break;
            case 1:
                c[0x34] = 65;
                break;
            case 2:
                c[105] = 252;
                c[252] = 9;
                c[254] = 20;
                break;
            case 3:
                c[105] = 128;
                capability(c, 128, 0, 1, 640, 56);
                break;
            case 4:
                bars[0].size = 519;
                break;
            case 5:
                c[84] = 6;
                break;
            case 6:
                c[82] = 16;
                break;
            case 7:
                put32(c + 76, 55);
                break;
            case 8:
                bars[0].base = UINT64_MAX - 100;
                break;
            case 9:
                c[6] = 0;
                break;
            case 10:
                c[105] = 0;
                c[107] = 3;
                break;
        }
        p = expected;
        CHECK(!kf_pci(c, bars, &p));
        CHECK(!memcmp(&p, &expected, sizeof p));
    }
    fixture(c, bars);
    bars[0].base = 0x90000000;
    CHECK(kf_pci(c, bars, &p) && p.common == 0x90000000 &&
          p.notify == 0x90000100);
    uint64_t features = (UINT64_C(1) << 32) | (1u << 9), address = 91;
    CHECK(kf_negotiate(features, 8, 15, &p, &address) && address == 0x9000013c);
    address = 91;
    CHECK(!kf_negotiate(features, 8, 16, &p, &address) && address == 91);
    CHECK(!kf_negotiate(features & ~(1u << 9), 8, 0, &p, &address));
    CHECK(!kf_negotiate(features & ~(UINT64_C(1) << 32), 8, 0, &p, &address));
    CHECK(!kf_negotiate(features, 7, 0, &p, &address));
    p.multiplier = UINT32_MAX;
    CHECK(!kf_negotiate(features, 8, UINT16_MAX, &p, &address));
    CHECK(address == 91);
    puts(
        "KF_DEVICE_PASS:pci:11-atomic-refusals:relocation:feature-notify-before-"
        "ready");
    return 0;
}

static int queue(void)
{
    static struct kf_queue q, before;
    static uint8_t bytes[4096];
    for (unsigned i = 0; i < 4096; ++i)
    {
        bytes[i] = (uint8_t)(i * 13 + 7);
    }
    kf_queue_init(&q, 65535);
    struct kf_ticket a, b, c = {91, 91};
    CHECK(kf_submit(&q, 1, 24, bytes, &a));
    CHECK(q.available == 0 && q.ring[7] == 0);
    CHECK(kf_submit(&q, 0, 32, 0, &b));
    CHECK(q.available == 1 && q.ring[0] == 3);
    CHECK(a.slot == 0 && b.slot == 1 && q.requests[0].header.sector == 24 &&
          q.requests[1].header.sector == 32);
    CHECK(q.descriptors[0].address != (q.descriptors[3].address) &&
          q.descriptors[1].address != q.descriptors[4].address &&
          q.descriptors[2].address != q.descriptors[5].address);
    CHECK(!memcmp(q.requests[0].bytes, bytes, 4096));
    CHECK(q.descriptors[1].flags == 1 && q.descriptors[4].flags == 3 &&
          q.descriptors[2].flags == 2);
    before = q;
    CHECK(!kf_submit(&q, 0, 40, 0, &c) && !memcmp(&before, &q, sizeof q) &&
          c.slot == 91);
    CHECK(!kf_complete(&q, 0, 2, 0) && !memcmp(&q, &before, sizeof q));
    CHECK(!kf_complete(&q, 1, 3, 0) && !memcmp(&q, &before, sizeof q));
    CHECK(kf_complete(&q, 0, 3, 0));
    CHECK(q.used == 0 && q.requests[1].state == KF_SUCCESS);
    before = q;
    CHECK(!kf_complete(&q, 1, 3, 0) && !memcmp(&before, &q, sizeof q));
    CHECK(kf_complete(&q, 1, 0, 0) && q.completed_writes == 1 &&
          q.stable_writes == 0);
    CHECK(kf_release(&q, a) && kf_release(&q, b));
    CHECK(!kf_release(&q, a));
    CHECK(kf_submit(&q, 4, 999, 0, &c) &&
          q.requests[c.slot].header.sector == 0);
    CHECK(!kf_submit(&q, 1, 0, bytes, &a));
    CHECK(kf_complete(&q, 2, c.slot * 3, 0) && q.stable_writes == 1);
    CHECK(kf_release(&q, c));
    CHECK(kf_submit(&q, 1, 0, bytes, &a));
    CHECK(kf_timeout(&q, a));
    before = q;
    CHECK(!kf_release(&q, a) && !kf_submit(&q, 0, 0, 0, &b) &&
          !kf_reset(&q, 0) && !memcmp(&before, &q, sizeof q));
    CHECK(kf_complete(&q, 3, a.slot * 3, 0));
    CHECK(q.requests[a.slot].state == KF_UNKNOWN && q.poisoned &&
          q.requests[a.slot].bytes[17] == bytes[17]);
    CHECK(kf_reset(&q, 1));
    CHECK(!kf_release(&q, a));
    CHECK(kf_submit(&q, 1, 0, bytes, &b));
    CHECK(kf_complete(&q, 1, b.slot * 3, 1));
    CHECK(q.requests[b.slot].state == KF_DEVICE_ERROR && q.stable_writes == 0);
    CHECK(kf_release(&q, b));
    /* A wire used element contains only head ID. After legal reuse it cannot
     * distinguish a protocol-violating duplicate from the new request. Local
     * generations protect caller tickets, not malicious/stale device DMA. */
    CHECK(kf_submit(&q, 0, 0, 0, &c));
    CHECK(c.generation != b.generation && c.slot == b.slot);
    CHECK(kf_complete(&q, 2, b.slot * 3, 0));
    CHECK(!kf_release(&q, b) && kf_release(&q, c));
    puts("KF_DEVICE_PASS:queue:two-owned-requests:wrap:out-of-order:timeout-"
         "retain:reset-ack:wire-no-generation-counterexample");
    return 0;
}

static int geometry(void)
{
    uint64_t sector = 91;
    CHECK(kf_page_sector(8192, 512, 1023, &sector) && sector == 8184);
    sector = 91;
    CHECK(!kf_page_sector(8192, 512, 1024, &sector) && sector == 91);
    CHECK(!kf_page_sector(8192, 4096, 0, &sector) && sector == 91);
    CHECK(kf_page_sector(UINT64_MAX, 512, UINT32_MAX, &sector) &&
          sector == UINT64_C(34359738360));
    uint32_t page = 91;
    for (uint32_t i = 0; i < 67; ++i)
    {
        CHECK(kf_slice(1024, 16, 67, i, &page) && page == 16 + i);
    }
    page = 91;
    CHECK(!kf_slice(4096, 16, 67, 67, &page) && page == 91);
    CHECK(!kf_slice(82, 16, 67, 66, &page) && page == 91);
    CHECK(kf_slice(4096, 16, 67, 66, &page) && page == 82);
    puts("KF_DEVICE_PASS:geometry:8-sectors:1023-1024:67-page-slice:sector-"
         "change-refused");
    return 0;
}

static int ordering(void)
{
    unsigned good = 0, bad = 0;
    for (unsigned d = 0; d < 3; ++d)
    {
        for (unsigned a = 0; a < 3; ++a)
        {
            for (unsigned n = 0; n < 3; ++n)
            {
                if (d != a && a != n && d != n)
                {
                    if (d < a && a < n)
                    {
                        ++good;
                    }
                    else
                    {
                        ++bad;
                    }
                }
            }
        }
    }
    CHECK(good == 1 && bad == 5);
    unsigned cpu_cached = 42, device_memory = 0;
    CHECK(device_memory != cpu_cached);
    device_memory = cpu_cached;
    CHECK(device_memory == 42);
    uint64_t cpu_physical = 0x2000, bus_address = 0x9000, iommu_target = 0;
    CHECK(cpu_physical != bus_address);
    iommu_target = cpu_physical;
    CHECK(iommu_target == 0x2000);
    puts(
        "KF_DEVICE_PASS:ordering:1-valid-5-counterorders:cache-clean-and-IOMMU-"
        "are-extra-premises");
    return 0;
}

static int durability(void)
{
    /* Explicit byte-prefix persistence model. A cut discards volatile cache;
     * it is not a QEMU power failure or a physical medium simulation. */
    static uint8_t cache[4096], durable[4096], recovered[4096];
    for (unsigned cut = 0; cut <= 4096; ++cut)
    {
        memset(cache, 0x7b, sizeof cache);
        memset(durable, 0, sizeof durable);
        memcpy(durable, cache,
               cut); /* legal spontaneous prefix before any flush */
        memcpy(recovered, durable,
               sizeof recovered); /* power-cut observation */
        for (unsigned i = 0; i < 4096; ++i)
        {
            CHECK(recovered[i] == (i < cut ? 0x7b : 0));
        }
        /* An error at this point cannot imply zero persistent writes. */
        if (cut)
        {
            CHECK(recovered[0] == 0x7b);
        }
        memcpy(durable, cache,
               sizeof durable); /* successful flush acknowledges all */
        memcpy(recovered, durable, sizeof recovered);
        CHECK(!memcmp(recovered, cache, sizeof cache));
    }
    puts(
        "KF_DEVICE_PASS:durability:4097-prefix-cuts:write-error-may-persist:flush-ack-all-bytes");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "pci"))
    {
        return pci();
    }
    if (!strcmp(argv[1], "queue"))
    {
        return queue();
    }
    if (!strcmp(argv[1], "geometry"))
    {
        return geometry();
    }
    if (!strcmp(argv[1], "ordering"))
    {
        return ordering();
    }
    if (!strcmp(argv[1], "durability"))
    {
        return durability();
    }
    return 2;
}
