#ifndef CMASTERY_SMP_DEPTH_ACPI_CHECKS_H
#define CMASTERY_SMP_DEPTH_ACPI_CHECKS_H
#include "acpi.h"

static void sa_w32(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(v >> (8 * i));
}

static void sa_sum(unsigned char *p, size_t n)
{
    unsigned sum = 0;
    p[9] = 0;
    for (size_t i = 0; i < n; ++i)
        sum = (sum + p[i]) & 255;
    p[9] = (unsigned char)(0u - sum);
}

static void sa_base(unsigned char *p, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        p[i] = 0;
    p[0] = 'A';
    p[1] = 'P';
    p[2] = 'I';
    p[3] = 'C';
    p[8] = 7;
    sa_w32(p + 4, (uint32_t)n);
    sa_w32(p + 36, 0xfee00000u);
}

static int sa_same(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; ++i)
        if (x[i] != y[i])
            return 0;
    return 1;
}

#define SAC(x)               \
    do                       \
    {                        \
        if (!(x))            \
            return __LINE__; \
    } while (0)

static int sa_checks(void)
{
    unsigned char p[200];
    sa_base(p, 76);
    for (unsigned i = 0; i < 4; ++i)
    {
        unsigned at = 44 + 8 * i;
        p[at] = 0;
        p[at + 1] = 8;
        p[at + 2] = (unsigned char)(10 + i);
        p[at + 3] = (unsigned char)(2 * i);
        sa_w32(p + at + 4, i == 2 ? 0u : 1u);
    }
    sa_sum(p, 76);
    struct sa_topology t = {0}, old = {0};
    SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_OK);
    SAC(t.count == 3 && t.disabled == 1 && t.cpu[0].apic == 0 &&
        t.cpu[1].apic == 2 && t.cpu[2].apic == 6 && t.cpu[2].uid == 13);
    old = t;
    unsigned char unaligned[80];
    for (unsigned i = 0; i < 76; ++i)
        unaligned[i + 1] = p[i];
    unaligned[77] = 255;
    unaligned[78] = 127;
    SAC(sa_parse(unaligned + 1, 78, 16, SA_XAPIC, &t) == SA_OK &&
        t.count == 3 && t.cpu[2].apic == 6);
    for (unsigned n = 0; n < 76; ++n)
    {
        SAC(sa_parse(p, n, 16, SA_XAPIC, &t) != SA_OK);
        SAC(sa_same(&t, &old, sizeof t));
    }
    p[10] ^= 1;
    SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_CHECKSUM);
    p[10] ^= 1;
    for (unsigned n = 0; n < 2; ++n)
    {
        p[45] = (unsigned char)n;
        sa_sum(p, 76);
        SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_INVALID);
        SAC(sa_same(&t, &old, sizeof t));
    }
    p[45] = 8;
    p[55] = 0;
    sa_sum(p, 76);
    SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_DUPLICATE);
    p[55] = 2;
    p[54] = 10;
    sa_sum(p, 76);
    SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_DUPLICATE);
    p[54] = 11;
    sa_sum(p, 76);
    SAC(sa_parse(p, 76, 2, SA_XAPIC, &t) == SA_FULL);
    SAC(sa_same(&t, &old, sizeof t));
    sa_w32(p + 4, 77);
    SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_INVALID);
    sa_w32(p + 4, 76);
    for (unsigned i = 0; i < 4; ++i)
        sa_w32(p + 48 + 8 * i, 0);
    sa_sum(p, 76);
    SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_EMPTY);
    sa_w32(p + 48, 1);
    sa_w32(p + 56, 2);
    sa_sum(p, 76);
    SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_OK && t.count == 1 &&
        t.online_capable == 1 && t.disabled == 2);
    sa_base(p, 76);
    p[44] = 9;
    p[45] = 16;
    sa_w32(p + 48, 0x12345678u);
    sa_w32(p + 52, 1);
    sa_w32(p + 56, 0xabcdef01u);
    p[60] = 9;
    p[61] = 16;
    sa_w32(p + 64, 0x23456789u);
    sa_w32(p + 68, 1);
    sa_w32(p + 72, 0xabcdef02u);
    sa_sum(p, 76);
    SAC(sa_parse(p, 76, 16, SA_XAPIC, &t) == SA_UNSUPPORTED);
    SAC(sa_parse(p, 76, 16, SA_X2_RECORDS, &t) == SA_OK);
    SAC(t.count == 2 && t.cpu[0].apic == 0x12345678u &&
        t.cpu[0].uid == 0xabcdef01u && t.cpu[1].apic == 0x23456789u);
    old = t;
    p[44] = 5;
    sa_sum(p, 76);
    SAC(sa_parse(p, 76, 16, SA_X2_RECORDS, &t) == SA_UNSUPPORTED &&
        sa_same(&t, &old, sizeof t));
    p[44] = 9;
    sa_w32(p + 4, 78);
    p[76] = 0x80;
    p[77] = 2;
    sa_sum(p, 78);
    SAC(sa_parse(p, 78, 16, SA_X2_RECORDS, &t) == SA_OK && t.unknown == 1);
    SAC(sa_parse(0, 78, 16, SA_XAPIC, &t) == SA_INVALID);
    SAC(sa_parse(p, 78, 0, SA_XAPIC, &t) == SA_INVALID);
    SAC(sa_parse(p, 78, 17, SA_XAPIC, &t) == SA_INVALID);
    SAC(sa_parse(p, 78, 16, 4, &t) == SA_INVALID);
    SAC(sa_parse(p, 78, 16, SA_XAPIC, 0) == SA_INVALID);
    return 0;
}
#endif
